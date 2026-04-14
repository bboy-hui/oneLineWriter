#if defined(_WIN32)

#include "Viewport3DWindow.hpp"

#include <GL/gl.h>

#include <algorithm>
#include <cmath>
#include <numbers>

#include <imgui.h>
#include <windowsx.h>

namespace owl::desktop
{
namespace
{
constexpr wchar_t kViewportWindowClass[] = L"OWLViewport3DWindow";

struct Vec3f
{
    float x {};
    float y {};
    float z {};
};

Vec3f ToVec(const owl::render::Vec3& v)
{
    return {v.x, v.y, v.z};
}

Vec3f operator+(const Vec3f& a, const Vec3f& b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3f operator-(const Vec3f& a, const Vec3f& b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3f operator*(const Vec3f& a, float s)
{
    return {a.x * s, a.y * s, a.z * s};
}

float Dot(const Vec3f& a, const Vec3f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3f Cross(const Vec3f& a, const Vec3f& b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

float Length(const Vec3f& v)
{
    return std::sqrt(Dot(v, v));
}

Vec3f Normalize(const Vec3f& v)
{
    const float len = Length(v);
    if (len < 1e-6f)
    {
        return {0.0f, 0.0f, 0.0f};
    }
    return v * (1.0f / len);
}

struct Mat4
{
    float m[16] {};
};

Mat4 LookAt(const Vec3f& eye, const Vec3f& target, const Vec3f& up)
{
    const Vec3f f = Normalize(target - eye);
    const Vec3f s = Normalize(Cross(f, up));
    const Vec3f u = Cross(s, f);

    Mat4 out {};
    // Column-major layout for OpenGL.
    out.m[0] = s.x;
    out.m[4] = s.y;
    out.m[8] = s.z;
    out.m[12] = -Dot(s, eye);

    out.m[1] = u.x;
    out.m[5] = u.y;
    out.m[9] = u.z;
    out.m[13] = -Dot(u, eye);

    out.m[2] = -f.x;
    out.m[6] = -f.y;
    out.m[10] = -f.z;
    out.m[14] = Dot(f, eye);

    out.m[3] = 0.0f;
    out.m[7] = 0.0f;
    out.m[11] = 0.0f;
    out.m[15] = 1.0f;
    return out;
}

void DecodeAarrggbb(std::uint32_t rgba, float& r, float& g, float& b, float& a)
{
    a = static_cast<float>((rgba >> 24) & 0xFF) / 255.0f;
    r = static_cast<float>((rgba >> 16) & 0xFF) / 255.0f;
    g = static_cast<float>((rgba >> 8) & 0xFF) / 255.0f;
    b = static_cast<float>((rgba >> 0) & 0xFF) / 255.0f;
}

bool EnsureChildClassRegistered(HINSTANCE instance)
{
    WNDCLASSEXW wc {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = Viewport3DWindow::ChildWndProc;
    wc.hInstance = instance;
    wc.lpszClassName = kViewportWindowClass;
    wc.hCursor = ::LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW
    return ::RegisterClassExW(&wc) != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool SetupPixelFormat(HDC dc)
{
    PIXELFORMATDESCRIPTOR pfd {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    const int format = ::ChoosePixelFormat(dc, &pfd);
    if (format == 0)
    {
        return false;
    }
    if (!::SetPixelFormat(dc, format, &pfd))
    {
        return false;
    }
    return true;
}
}

Viewport3DWindow::~Viewport3DWindow()
{
    Shutdown();
}

bool Viewport3DWindow::Initialize(HWND parentHwnd, HDC parentDc, HGLRC parentGlrc)
{
    if (initialized_)
    {
        return true;
    }

    parentHwnd_ = parentHwnd;
    parentDc_ = parentDc;
    parentGlrc_ = parentGlrc;

    const HINSTANCE instance = ::GetModuleHandleW(nullptr);
    if (!EnsureChildClassRegistered(instance))
    {
        return false;
    }

    childHwnd_ = ::CreateWindowExW(
        0,
        kViewportWindowClass,
        L"Viewport3D",
        WS_CHILD,
        0,
        0,
        16,
        16,
        parentHwnd_,
        nullptr,
        instance,
        this);

    if (!childHwnd_)
    {
        return false;
    }

    childDc_ = ::GetDC(childHwnd_);
    if (!childDc_)
    {
        Shutdown();
        return false;
    }

    if (!SetupPixelFormat(childDc_))
    {
        Shutdown();
        return false;
    }

    childGlrc_ = ::wglCreateContext(childDc_);
    if (!childGlrc_)
    {
        Shutdown();
        return false;
    }

    if (parentGlrc_ != nullptr)
    {
        (void)::wglShareLists(parentGlrc_, childGlrc_);
    }

    ::ShowWindow(childHwnd_, SW_HIDE);
    visible_ = false;
    ResetCamera();
    initialized_ = true;
    return true;
}

void Viewport3DWindow::Shutdown()
{
    if (!initialized_)
    {
        return;
    }

    if (childGlrc_)
    {
        ::wglMakeCurrent(nullptr, nullptr);
        ::wglDeleteContext(childGlrc_);
        childGlrc_ = nullptr;
    }

    if (childDc_ && childHwnd_)
    {
        ::ReleaseDC(childHwnd_, childDc_);
        childDc_ = nullptr;
    }

    if (childHwnd_)
    {
        ::DestroyWindow(childHwnd_);
        childHwnd_ = nullptr;
    }

    initialized_ = false;
    visible_ = false;
    polylines_.clear();
}

void Viewport3DWindow::DrawImGuiPanel(std::span<const owl::render::Polyline3D> polylines)
{
    if (!initialized_ || childHwnd_ == nullptr)
    {
        return;
    }

    wantRender_ = false;
    desiredVisible_ = false;

    ImGui::Begin("3D Viewport");
    ImGui::Checkbox("Auto fit", &autoFit_);
    ImGui::SameLine();
    if (ImGui::Button("Reset camera"))
    {
        ResetCamera();
    }

    ImGui::SliderFloat("Yaw", &yawDeg_, -180.0f, 180.0f, "%.1f deg");
    ImGui::SliderFloat("Pitch", &pitchDeg_, -89.0f, 89.0f, "%.1f deg");
    ImGui::SliderFloat("Distance", &distance_, 10.0f, 800.0f, "%.1f");
    ImGui::TextUnformatted("Mouse: L-drag rotate, wheel zoom (focus viewport).");

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const ImVec2 reservedSize {std::max(avail.x, 64.0f), std::max(avail.y, 64.0f)};
    ImGui::InvisibleButton("viewport3d_host", reservedSize);

    const bool shouldShow = ImGui::IsItemVisible() && !ImGui::IsWindowCollapsed();
    const ImVec2 rectMin = ImGui::GetItemRectMin();
    const ImVec2 rectMax = ImGui::GetItemRectMax();

    desiredRect_.left = static_cast<LONG>(rectMin.x);
    desiredRect_.top = static_cast<LONG>(rectMin.y);
    desiredRect_.right = static_cast<LONG>(rectMax.x);
    desiredRect_.bottom = static_cast<LONG>(rectMax.y);
    placementDirty_ = true;

    if (shouldShow)
    {
        desiredVisible_ = true;
        polylines_.assign(polylines.begin(), polylines.end());
        wantRender_ = true;
    }
    else
    {
        polylines_.clear();
    }

    ImGui::End();

    UpdatePlacementFromImGui();

    if (!desiredVisible_ && visible_)
    {
        ::ShowWindow(childHwnd_, SW_HIDE);
        visible_ = false;
    }

    if (autoFit_)
    {
        UpdateBounds();
        FitCameraToContent();
    }

    if (wantRender_)
    {
        RenderFrame();
    }
}

void Viewport3DWindow::UpdatePlacementFromImGui()
{
    if (!placementDirty_ || childHwnd_ == nullptr || parentHwnd_ == nullptr)
    {
        return;
    }
    placementDirty_ = false;

    if (!desiredVisible_)
    {
        return;
    }

    POINT tl {desiredRect_.left, desiredRect_.top};
    POINT br {desiredRect_.right, desiredRect_.bottom};
    ::ScreenToClient(parentHwnd_, &tl);
    ::ScreenToClient(parentHwnd_, &br);

    const int w = std::max(1, static_cast<int>(br.x - tl.x));
    const int h = std::max(1, static_cast<int>(br.y - tl.y));
    width_ = static_cast<std::uint32_t>(w);
    height_ = static_cast<std::uint32_t>(h);

    ::SetWindowPos(childHwnd_, nullptr, tl.x, tl.y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
    if (!visible_)
    {
        ::ShowWindow(childHwnd_, SW_SHOW);
        visible_ = true;
    }
}

void Viewport3DWindow::RenderFrame()
{
    if (!initialized_ || !visible_ || childDc_ == nullptr || childGlrc_ == nullptr)
    {
        return;
    }

    (void)::wglMakeCurrent(childDc_, childGlrc_);
    RenderSceneFixedFunction();
    ::SwapBuffers(childDc_);

    if (parentDc_ != nullptr && parentGlrc_ != nullptr)
    {
        (void)::wglMakeCurrent(parentDc_, parentGlrc_);
    }
}

void Viewport3DWindow::RenderSceneFixedFunction()
{
    const float clearR = 0.06f;
    const float clearG = 0.06f;
    const float clearB = 0.08f;

    ::glViewport(0, 0, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
    ::glClearColor(clearR, clearG, clearB, 1.0f);
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ::glEnable(GL_DEPTH_TEST);
    ::glDisable(GL_CULL_FACE);
    ::glDisable(GL_LIGHTING);

    const float aspect = height_ > 0 ? static_cast<float>(width_) / static_cast<float>(height_) : 1.0f;
    const float nearZ = 0.5f;
    const float farZ = 5000.0f;
    const float fovYRad = 45.0f * std::numbers::pi_v<float> / 180.0f;
    const float top = std::tan(fovYRad * 0.5f) * nearZ;
    const float right = top * aspect;

    ::glMatrixMode(GL_PROJECTION);
    ::glLoadIdentity();
    ::glFrustum(-right, right, -top, top, nearZ, farZ);

    const float yawRad = yawDeg_ * std::numbers::pi_v<float> / 180.0f;
    const float pitchRad = pitchDeg_ * std::numbers::pi_v<float> / 180.0f;

    const float cp = std::cos(pitchRad);
    const Vec3f forward {std::cos(yawRad) * cp, std::sin(yawRad) * cp, std::sin(pitchRad)};
    const Vec3f target = ToVec(target_);
    const Vec3f eye = target - forward * distance_;
    const Mat4 view = LookAt(eye, target, {0.0f, 0.0f, 1.0f});

    ::glMatrixMode(GL_MODELVIEW);
    ::glLoadMatrixf(view.m);

    // Grid on XY plane.
    ::glLineWidth(1.0f);
    ::glColor4f(0.20f, 0.20f, 0.22f, 1.0f);
    ::glBegin(GL_LINES);
    constexpr int gridLines = 20;
    constexpr float gridStep = 10.0f;
    for (int i = -gridLines; i <= gridLines; ++i)
    {
        const float v = static_cast<float>(i) * gridStep;
        ::glVertex3f(v, -gridLines * gridStep, 0.0f);
        ::glVertex3f(v, gridLines * gridStep, 0.0f);
        ::glVertex3f(-gridLines * gridStep, v, 0.0f);
        ::glVertex3f(gridLines * gridStep, v, 0.0f);
    }
    ::glEnd();

    // Axes.
    ::glLineWidth(2.0f);
    ::glBegin(GL_LINES);
    ::glColor4f(1.0f, 0.3f, 0.3f, 1.0f);
    ::glVertex3f(0.0f, 0.0f, 0.0f);
    ::glVertex3f(50.0f, 0.0f, 0.0f);
    ::glColor4f(0.3f, 1.0f, 0.3f, 1.0f);
    ::glVertex3f(0.0f, 0.0f, 0.0f);
    ::glVertex3f(0.0f, 50.0f, 0.0f);
    ::glColor4f(0.3f, 0.6f, 1.0f, 1.0f);
    ::glVertex3f(0.0f, 0.0f, 0.0f);
    ::glVertex3f(0.0f, 0.0f, 50.0f);
    ::glEnd();

    // Simple cube at origin.
    ::glColor4f(0.9f, 0.8f, 0.3f, 1.0f);
    ::glLineWidth(1.5f);
    ::glBegin(GL_LINES);
    constexpr float s = 5.0f;
    const Vec3f v0 {-s, -s, -s};
    const Vec3f v1 {s, -s, -s};
    const Vec3f v2 {s, s, -s};
    const Vec3f v3 {-s, s, -s};
    const Vec3f v4 {-s, -s, s};
    const Vec3f v5 {s, -s, s};
    const Vec3f v6 {s, s, s};
    const Vec3f v7 {-s, s, s};
    auto edge = [](const Vec3f& a, const Vec3f& b) {
        ::glVertex3f(a.x, a.y, a.z);
        ::glVertex3f(b.x, b.y, b.z);
    };
    edge(v0, v1);
    edge(v1, v2);
    edge(v2, v3);
    edge(v3, v0);
    edge(v4, v5);
    edge(v5, v6);
    edge(v6, v7);
    edge(v7, v4);
    edge(v0, v4);
    edge(v1, v5);
    edge(v2, v6);
    edge(v3, v7);
    ::glEnd();

    // Polylines.
    for (const auto& polyline : polylines_)
    {
        if (polyline.points.size() < 2)
        {
            continue;
        }

        float r {};
        float g {};
        float b {};
        float a {};
        DecodeAarrggbb(polyline.rgba, r, g, b, a);
        ::glColor4f(r, g, b, std::clamp(a, 0.2f, 1.0f));
        ::glLineWidth(polyline.layer == owl::render::Polyline3D::Layer::Process ? 2.5f : 1.5f);

        if (polyline.layer == owl::render::Polyline3D::Layer::Travel)
        {
            ::glEnable(GL_LINE_STIPPLE);
            ::glLineStipple(1, 0x00FF);
        }
        else
        {
            ::glDisable(GL_LINE_STIPPLE);
        }

        ::glBegin(GL_LINE_STRIP);
        for (const auto& p : polyline.points)
        {
            ::glVertex3f(p.x, p.y, p.z);
        }
        ::glEnd();
    }

    ::glDisable(GL_LINE_STIPPLE);
}

void Viewport3DWindow::ResetCamera()
{
    yawDeg_ = 45.0f;
    pitchDeg_ = 30.0f;
    distance_ = 120.0f;
    target_ = {};
}

void Viewport3DWindow::UpdateBounds()
{
    bounds_ = {};
    float minX = 0.0f;
    float minY = 0.0f;
    float minZ = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;
    bool hasPoint = false;

    for (const auto& polyline : polylines_)
    {
        for (const auto& p : polyline.points)
        {
            if (!hasPoint)
            {
                minX = maxX = p.x;
                minY = maxY = p.y;
                minZ = maxZ = p.z;
                hasPoint = true;
                continue;
            }
            minX = std::min(minX, p.x);
            minY = std::min(minY, p.y);
            minZ = std::min(minZ, p.z);
            maxX = std::max(maxX, p.x);
            maxY = std::max(maxY, p.y);
            maxZ = std::max(maxZ, p.z);
        }
    }

    if (!hasPoint)
    {
        bounds_.valid = false;
        return;
    }

    bounds_.valid = true;
    bounds_.min = {minX, minY, minZ};
    bounds_.max = {maxX, maxY, maxZ};
}

void Viewport3DWindow::FitCameraToContent()
{
    if (!bounds_.valid)
    {
        target_ = {};
        return;
    }

    target_.x = (bounds_.min.x + bounds_.max.x) * 0.5f;
    target_.y = (bounds_.min.y + bounds_.max.y) * 0.5f;
    target_.z = (bounds_.min.z + bounds_.max.z) * 0.5f;

    const float dx = bounds_.max.x - bounds_.min.x;
    const float dy = bounds_.max.y - bounds_.min.y;
    const float dz = bounds_.max.z - bounds_.min.z;
    const float radius = 0.5f * std::sqrt(dx * dx + dy * dy + dz * dz);

    const float minDist = 15.0f;
    distance_ = std::max(minDist, radius * 2.2f + 20.0f);
}

LRESULT CALLBACK Viewport3DWindow::ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* self = static_cast<Viewport3DWindow*>(create->lpCreateParams);
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        return ::DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    auto* self = reinterpret_cast<Viewport3DWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (self)
    {
        return self->HandleMessage(hwnd, msg, wParam, lParam);
    }
    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Viewport3DWindow::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE: {
        const int w = LOWORD(lParam);
        const int h = HIWORD(lParam);
        width_ = static_cast<std::uint32_t>(std::max(1, w));
        height_ = static_cast<std::uint32_t>(std::max(1, h));
        return 0;
    }
    case WM_LBUTTONDOWN: {
        mouseDragging_ = true;
        lastMousePos_.x = GET_X_LPARAM(lParam);
        lastMousePos_.y = GET_Y_LPARAM(lParam);
        ::SetCapture(hwnd);
        return 0;
    }
    case WM_LBUTTONUP: {
        mouseDragging_ = false;
        ::ReleaseCapture();
        return 0;
    }
    case WM_MOUSEMOVE: {
        if (!mouseDragging_)
        {
            return 0;
        }
        const int x = GET_X_LPARAM(lParam);
        const int y = GET_Y_LPARAM(lParam);
        const int dx = x - lastMousePos_.x;
        const int dy = y - lastMousePos_.y;
        lastMousePos_.x = x;
        lastMousePos_.y = y;

        yawDeg_ += static_cast<float>(dx) * 0.25f;
        pitchDeg_ -= static_cast<float>(dy) * 0.25f;
        pitchDeg_ = std::clamp(pitchDeg_, -89.0f, 89.0f);
        autoFit_ = false;
        return 0;
    }
    case WM_MOUSEWHEEL: {
        const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
        const float zoom = delta > 0 ? 0.92f : 1.08f;
        distance_ = std::clamp(distance_ * zoom, 5.0f, 5000.0f);
        autoFit_ = false;
        return 0;
    }
    default:
        return ::DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
}

#endif
