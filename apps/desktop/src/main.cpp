#include <iostream>
#include <limits>
#include <string_view>
#include <utility>

#include <imgui.h>

#include "DesktopWorkbench.hpp"
#include "owl/core/Application.hpp"
#include "owl/device/GrblController.hpp"
#include "owl/render/RenderBackendFactory.hpp"
#include "owl/render/RenderService.hpp"

#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
struct WindowState
{
    HWND hWnd {};
    HDC hDC {};
    HGLRC hGLRC {};
};

WindowState g_window;

owl::render::BackendType ParseBackend(int argc, char** argv)
{
    for (int index = 1; index < argc; ++index)
    {
        const std::string_view arg = argv[index];
        if (arg == "--backend=vulkan")
        {
            return owl::render::BackendType::Vulkan;
        }
        if (arg == "--backend=opengl")
        {
            return owl::render::BackendType::OpenGL;
        }
    }
    return owl::render::BackendType::OpenGL;
}

void DrawPathPreview2D(const std::vector<owl::render::Polyline3D>& polylines)
{
    ImGui::Begin("Path Preview 2D");
    const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    const float canvasWidth = canvasSize.x > 100.0f ? canvasSize.x : 100.0f;
    const float canvasHeight = canvasSize.y > 120.0f ? canvasSize.y : 120.0f;
    const ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("path_canvas", ImVec2(canvasWidth, canvasHeight));

    auto* drawList = ImGui::GetWindowDrawList();
    const ImU32 borderColor = IM_COL32(180, 180, 180, 255);
    const ImU32 bgColor = IM_COL32(24, 24, 28, 255);
    drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasWidth, canvasPos.y + canvasHeight), bgColor);
    drawList->AddRect(canvasPos, ImVec2(canvasPos.x + canvasWidth, canvasPos.y + canvasHeight), borderColor);

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    bool hasPoint = false;
    for (const auto& polyline : polylines)
    {
        for (const auto& point : polyline.points)
        {
            minX = point.x < minX ? point.x : minX;
            minY = point.y < minY ? point.y : minY;
            maxX = point.x > maxX ? point.x : maxX;
            maxY = point.y > maxY ? point.y : maxY;
            hasPoint = true;
        }
    }

    if (hasPoint)
    {
        const float pad = 20.0f;
        const float worldW = (maxX - minX) > 1e-4f ? (maxX - minX) : 1.0f;
        const float worldH = (maxY - minY) > 1e-4f ? (maxY - minY) : 1.0f;
        const float sx = (canvasWidth - pad * 2.0f) / worldW;
        const float sy = (canvasHeight - pad * 2.0f) / worldH;
        const float scale = sx < sy ? sx : sy;

        const float usedW = worldW * scale;
        const float usedH = worldH * scale;
        const float offsetX = canvasPos.x + (canvasWidth - usedW) * 0.5f;
        const float offsetY = canvasPos.y + (canvasHeight - usedH) * 0.5f;

        auto toScreen = [&](const owl::render::Vec3& p) {
            const float px = offsetX + (p.x - minX) * scale;
            const float py = offsetY + usedH - (p.y - minY) * scale;
            return ImVec2(px, py);
        };

        for (const auto& polyline : polylines)
        {
            if (polyline.points.size() < 2)
            {
                continue;
            }

            const bool isProcess = polyline.layer == owl::render::Polyline3D::Layer::Process;
            const ImU32 color = isProcess ? IM_COL32(80, 220, 255, 255) : IM_COL32(150, 150, 150, 255);
            const float thickness = isProcess ? 2.0f : 1.0f;

            for (std::size_t i = 1; i < polyline.points.size(); ++i)
            {
                drawList->AddLine(toScreen(polyline.points[i - 1]), toScreen(polyline.points[i]), color, thickness);
            }
        }
    }
    else
    {
        drawList->AddText(ImVec2(canvasPos.x + 12.0f, canvasPos.y + 12.0f), IM_COL32(210, 210, 210, 255), "No path data");
    }

    ImGui::Text("Legend:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.31f, 0.86f, 1.0f, 1.0f), "Process");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.62f, 0.62f, 0.62f, 1.0f), "Travel");
    ImGui::End();
}

bool CreateDeviceWGL(HWND hWnd)
{
    g_window.hDC = ::GetDC(hWnd);
    if (g_window.hDC == nullptr)
    {
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pixelFormat = ::ChoosePixelFormat(g_window.hDC, &pfd);
    if (pixelFormat == 0)
    {
        return false;
    }
    if (::SetPixelFormat(g_window.hDC, pixelFormat, &pfd) == FALSE)
    {
        return false;
    }

    g_window.hGLRC = ::wglCreateContext(g_window.hDC);
    if (g_window.hGLRC == nullptr)
    {
        return false;
    }
    if (::wglMakeCurrent(g_window.hDC, g_window.hGLRC) == FALSE)
    {
        return false;
    }
    return true;
}

void CleanupDeviceWGL()
{
    if (g_window.hGLRC != nullptr)
    {
        ::wglMakeCurrent(nullptr, nullptr);
        ::wglDeleteContext(g_window.hGLRC);
    }
    if (g_window.hWnd != nullptr && g_window.hDC != nullptr)
    {
        ::ReleaseDC(g_window.hWnd, g_window.hDC);
    }
    g_window.hGLRC = nullptr;
    g_window.hDC = nullptr;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }

    switch (msg)
    {
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            const int width = LOWORD(lParam);
            const int height = HIWORD(lParam);
            ::glViewport(0, 0, width, height);
        }
        return 0;
    default:
        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}
}

int main(int argc, char** argv)
{
    owl::core::Application app("OneLineWriter");
    app.Bootstrap();

    const auto backendType = ParseBackend(argc, argv);

    WNDCLASSEXW wc {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = ::GetModuleHandleW(nullptr);
    wc.lpszClassName = L"OneLineWriterWindow";
    ::RegisterClassExW(&wc);

    g_window.hWnd = ::CreateWindowW(
        wc.lpszClassName,
        L"OneLineWriter",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        1400,
        900,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

    if (g_window.hWnd == nullptr || !CreateDeviceWGL(g_window.hWnd))
    {
        return 1;
    }

    ::ShowWindow(g_window.hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(g_window.hWnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_InitForOpenGL(g_window.hWnd);
    ImGui_ImplOpenGL3_Init("#version 130");

    owl::desktop::DesktopWorkbench workbench;
    const bool exported = workbench.BuildAndExport();

    auto backend = owl::render::CreateRenderBackend(backendType);
    owl::render::RenderService renderService(std::move(backend));
    owl::render::RenderInitOptions renderInit {};
    renderInit.backend = backendType;
    renderInit.nativeWindowHandle = g_window.hWnd;
    renderInit.viewportWidth = 1400;
    renderInit.viewportHeight = 900;
    renderService.Initialize(renderInit);
    owl::render::CameraState camera {};

    owl::device::GrblController controller;
    controller.Connect("COM1", 115200);
    controller.QueueGCode(workbench.LastGCode());
    for (const auto& line : controller.FlushPending(1024))
    {
        std::cout << line << '\n';
    }
    std::cout << "Export: " << (exported ? "OK" : "FAILED") << " -> " << workbench.OutputPath() << '\n';
    std::cout << "Render backend: " << (backendType == owl::render::BackendType::Vulkan ? "Vulkan" : "OpenGL") << '\n';

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
            {
                done = true;
            }
        }
        if (done)
        {
            break;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        workbench.DrawPanels();
        auto previewPolylines = workbench.BuildPreviewPolylines();
        renderService.Draw(camera, previewPolylines);
        const auto frameStats = renderService.LastFrameStats();
        DrawPathPreview2D(previewPolylines);

        ImGui::Begin("Render Stats");
        ImGui::Text("Backend: %s", backendType == owl::render::BackendType::Vulkan ? "Vulkan" : "OpenGL");
        ImGui::Text("Polylines: %d", static_cast<int>(frameStats.polylineCount));
        ImGui::Text("Process Polylines: %d", static_cast<int>(frameStats.processPolylineCount));
        ImGui::Text("Travel Polylines: %d", static_cast<int>(frameStats.travelPolylineCount));
        ImGui::Text("Vertices: %d", static_cast<int>(frameStats.vertexCount));
        ImGui::Text("Bounds Valid: %s", frameStats.bounds.valid ? "true" : "false");
        if (frameStats.bounds.valid)
        {
            ImGui::Text("Bounds Min: (%.3f, %.3f, %.3f)", frameStats.bounds.min.x, frameStats.bounds.min.y, frameStats.bounds.min.z);
            ImGui::Text("Bounds Max: (%.3f, %.3f, %.3f)", frameStats.bounds.max.x, frameStats.bounds.max.y, frameStats.bounds.max.z);
        }
        ImGui::End();

        ImGui::Render();
        ::glViewport(0, 0, static_cast<int>(ImGui::GetIO().DisplaySize.x), static_cast<int>(ImGui::GetIO().DisplaySize.y));
        ::glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        ::glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ::SwapBuffers(g_window.hDC);
    }

    controller.Disconnect();
    renderService.Shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceWGL();
    ::DestroyWindow(g_window.hWnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return exported ? 0 : 1;
}
#else
int main()
{
    std::cout << "UI mode requires Windows backend in current build.\n";
    return 0;
}
#endif
