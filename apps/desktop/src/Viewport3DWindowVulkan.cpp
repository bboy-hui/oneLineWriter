#if defined(_WIN32)

#include "Viewport3DWindowVulkan.hpp"

#include <algorithm>
#include <cmath>

#include <imgui.h>
#include <windowsx.h>

namespace owl::desktop
{
namespace
{
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
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
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

void DecodeAarrggbb(std::uint32_t rgba, float& r, float& g, float& b, float& a)
{
    a = static_cast<float>((rgba >> 24) & 0xFF) / 255.0f;
    r = static_cast<float>((rgba >> 16) & 0xFF) / 255.0f;
    g = static_cast<float>((rgba >> 8) & 0xFF) / 255.0f;
    b = static_cast<float>((rgba >> 0) & 0xFF) / 255.0f;
}

struct CameraBasis
{
    Vec3f eye {};
    Vec3f forward {};
    Vec3f right {};
    Vec3f up {};
};

CameraBasis BuildCamera(float yawDeg, float pitchDeg, float distance, const owl::render::Vec3& target)
{
    constexpr float kPi = 3.14159265358979323846f;
    const float yawRad = yawDeg * (kPi / 180.0f);
    const float pitchRad = pitchDeg * (kPi / 180.0f);

    const float cp = std::cos(pitchRad);
    Vec3f forward {
        std::cos(yawRad) * cp,
        std::sin(yawRad) * cp,
        std::sin(pitchRad),
    };
    forward = Normalize(forward);

    const Vec3f worldUp = std::fabs(forward.z) > 0.98f ? Vec3f {0.0f, 1.0f, 0.0f} : Vec3f {0.0f, 0.0f, 1.0f};
    const Vec3f right = Normalize(Cross(forward, worldUp));
    const Vec3f up = Normalize(Cross(right, forward));

    const Vec3f targetPos = ToVec(target);
    const Vec3f eye = targetPos - forward * distance;

    return {eye, forward, right, up};
}

bool ProjectPoint(const CameraBasis& camera, const Vec3f& point, const ImVec2& canvasMin, const ImVec2& canvasMax, ImVec2& out)
{
    const Vec3f rel = point - camera.eye;
    const float x = Dot(rel, camera.right);
    const float y = Dot(rel, camera.up);
    const float z = Dot(rel, camera.forward);
    if (z <= 0.1f)
    {
        return false;
    }

    constexpr float kFovY = 45.0f;
    constexpr float kPi = 3.14159265358979323846f;
    const float tanHalfFov = std::tan((kFovY * (kPi / 180.0f)) * 0.5f);

    const float width = std::max(1.0f, canvasMax.x - canvasMin.x);
    const float height = std::max(1.0f, canvasMax.y - canvasMin.y);
    const float aspect = width / height;

    const float ndcX = x / (z * tanHalfFov * aspect);
    const float ndcY = y / (z * tanHalfFov);

    if (std::fabs(ndcX) > 4.0f || std::fabs(ndcY) > 4.0f)
    {
        return false;
    }

    out.x = canvasMin.x + (ndcX * 0.5f + 0.5f) * width;
    out.y = canvasMin.y + (-ndcY * 0.5f + 0.5f) * height;
    return true;
}

void DrawSegment(ImDrawList* drawList,
    const CameraBasis& camera,
    const Vec3f& a,
    const Vec3f& b,
    ImU32 color,
    float thickness)
{
    ImVec2 pa {};
    ImVec2 pb {};
    if (ProjectPoint(camera, a, drawList->GetClipRectMin(), drawList->GetClipRectMax(), pa)
        && ProjectPoint(camera, b, drawList->GetClipRectMin(), drawList->GetClipRectMax(), pb))
    {
        drawList->AddLine(pa, pb, color, thickness);
    }
}
}

Viewport3DWindowVulkan::~Viewport3DWindowVulkan()
{
    Shutdown();
}

bool Viewport3DWindowVulkan::Initialize(HWND parentHwnd)
{
    parentHwnd_ = parentHwnd;
    initialized_ = true;
    lastError_.clear();
    return true;
}

void Viewport3DWindowVulkan::Shutdown()
{
    if (!initialized_)
    {
        return;
    }

    ShutdownVulkan();
    DestroyChildWindow();

    initialized_ = false;
    visible_ = false;
    polylines_.clear();
}

void Viewport3DWindowVulkan::DrawImGuiPanel(std::span<const owl::render::Polyline3D> polylines)
{
    if (!initialized_)
    {
        return;
    }

    ImGui::Begin("3D Viewport");

    ImGui::Checkbox("Auto fit", &autoFit_);
    ImGui::SameLine();
    if (ImGui::Button("Reset camera"))
    {
        yawDeg_ = 45.0f;
        pitchDeg_ = 30.0f;
        distance_ = 120.0f;
        target_ = {};
        autoFit_ = true;
    }

    ImGui::SliderFloat("Yaw", &yawDeg_, -180.0f, 180.0f, "%.1f deg");
    ImGui::SliderFloat("Pitch", &pitchDeg_, -89.0f, 89.0f, "%.1f deg");
    ImGui::SliderFloat("Distance", &distance_, 10.0f, 8000.0f, "%.1f");
    ImGui::TextUnformatted("Mouse: L-drag rotate, wheel zoom.");

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const ImVec2 canvasSize {std::max(avail.x, 120.0f), std::max(avail.y, 120.0f)};
    ImGui::InvisibleButton("viewport3d_vulkan_canvas", canvasSize,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    const ImVec2 canvasMin = ImGui::GetItemRectMin();
    const ImVec2 canvasMax = ImGui::GetItemRectMax();
    const bool hovered = ImGui::IsItemHovered();

    if (hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        const ImVec2 delta = ImGui::GetIO().MouseDelta;
        yawDeg_ += delta.x * 0.25f;
        pitchDeg_ -= delta.y * 0.25f;
        pitchDeg_ = std::clamp(pitchDeg_, -89.0f, 89.0f);
        autoFit_ = false;
    }

    if (hovered)
    {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f)
        {
            const float zoom = wheel > 0.0f ? 0.92f : 1.08f;
            distance_ = std::clamp(distance_ * zoom, 5.0f, 10000.0f);
            autoFit_ = false;
        }
    }

    polylines_.assign(polylines.begin(), polylines.end());
    UpdateBounds();
    if (autoFit_)
    {
        FitCameraToContent();
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImU32 bgColor = IM_COL32(16, 18, 24, 255);
    const ImU32 borderColor = IM_COL32(110, 120, 140, 255);
    drawList->AddRectFilled(canvasMin, canvasMax, bgColor);
    drawList->AddRect(canvasMin, canvasMax, borderColor);

    drawList->PushClipRect(canvasMin, canvasMax, true);

    const CameraBasis camera = BuildCamera(yawDeg_, pitchDeg_, distance_, target_);

    float gridExtent = 120.0f;
    if (bounds_.valid)
    {
        const float dx = bounds_.max.x - bounds_.min.x;
        const float dy = bounds_.max.y - bounds_.min.y;
        gridExtent = std::max(60.0f, std::max(dx, dy) * 0.8f + 20.0f);
    }
    const float step = std::max(5.0f, gridExtent / 12.0f);

    const ImU32 gridColor = IM_COL32(60, 64, 72, 255);
    for (float v = -gridExtent; v <= gridExtent; v += step)
    {
        DrawSegment(drawList, camera, {-gridExtent, v, 0.0f}, {gridExtent, v, 0.0f}, gridColor, 1.0f);
        DrawSegment(drawList, camera, {v, -gridExtent, 0.0f}, {v, gridExtent, 0.0f}, gridColor, 1.0f);
    }

    const float axisLen = std::max(30.0f, gridExtent * 0.35f);
    DrawSegment(drawList, camera, {0.0f, 0.0f, 0.0f}, {axisLen, 0.0f, 0.0f}, IM_COL32(220, 90, 90, 255), 2.0f);
    DrawSegment(drawList, camera, {0.0f, 0.0f, 0.0f}, {0.0f, axisLen, 0.0f}, IM_COL32(90, 220, 120, 255), 2.0f);
    DrawSegment(drawList, camera, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, axisLen}, IM_COL32(90, 130, 240, 255), 2.0f);

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
        const ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, std::clamp(a, 0.25f, 1.0f)));
        const float thickness = polyline.layer == owl::render::Polyline3D::Layer::Process ? 2.2f : 1.2f;

        for (std::size_t i = 1; i < polyline.points.size(); ++i)
        {
            const Vec3f p0 = ToVec(polyline.points[i - 1]);
            const Vec3f p1 = ToVec(polyline.points[i]);
            DrawSegment(drawList, camera, p0, p1, color, thickness);
        }
    }

    drawList->PopClipRect();

    ImGui::Text("Polylines: %d", static_cast<int>(polylines_.size()));
    if (bounds_.valid)
    {
        ImGui::Text("Bounds min: (%.2f, %.2f, %.2f)", bounds_.min.x, bounds_.min.y, bounds_.min.z);
        ImGui::Text("Bounds max: (%.2f, %.2f, %.2f)", bounds_.max.x, bounds_.max.y, bounds_.max.z);
    }
    else
    {
        ImGui::TextUnformatted("No path data.");
    }
    if (!lastError_.empty())
    {
        ImGui::TextWrapped("Last error: %s", lastError_.c_str());
    }

    ImGui::End();
}

const std::string& Viewport3DWindowVulkan::LastError() const noexcept
{
    return lastError_;
}

LRESULT CALLBACK Viewport3DWindowVulkan::ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* self = static_cast<Viewport3DWindowVulkan*>(create->lpCreateParams);
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        return ::DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    auto* self = reinterpret_cast<Viewport3DWindowVulkan*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (self)
    {
        return self->HandleMessage(hwnd, msg, wParam, lParam);
    }
    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Viewport3DWindowVulkan::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
    default:
        return ::DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

void Viewport3DWindowVulkan::UpdatePlacementFromImGui()
{
}

bool Viewport3DWindowVulkan::EnsureChildWindow()
{
    return true;
}

void Viewport3DWindowVulkan::DestroyChildWindow()
{
}

bool Viewport3DWindowVulkan::InitializeVulkan()
{
    return true;
}

void Viewport3DWindowVulkan::ShutdownVulkan()
{
}

bool Viewport3DWindowVulkan::CreateOrResizeSwapchain()
{
    return true;
}

void Viewport3DWindowVulkan::DestroySwapchain()
{
}

bool Viewport3DWindowVulkan::CreateDepthResources()
{
    return true;
}

void Viewport3DWindowVulkan::DestroyDepthResources()
{
}

bool Viewport3DWindowVulkan::CreateRenderPass()
{
    return true;
}

void Viewport3DWindowVulkan::DestroyRenderPass()
{
}

bool Viewport3DWindowVulkan::CreatePipeline()
{
    return true;
}

void Viewport3DWindowVulkan::DestroyPipeline()
{
}

bool Viewport3DWindowVulkan::CreateFrameResources()
{
    return true;
}

void Viewport3DWindowVulkan::DestroyFrameResources()
{
}

bool Viewport3DWindowVulkan::RenderFrame()
{
    return true;
}

void Viewport3DWindowVulkan::RequestSwapchainRebuild()
{
    swapchainDirty_ = true;
}

void Viewport3DWindowVulkan::UpdateBounds()
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

void Viewport3DWindowVulkan::FitCameraToContent()
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
}

#endif
