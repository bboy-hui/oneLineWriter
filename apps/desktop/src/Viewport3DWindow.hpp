#pragma once

#if defined(_WIN32)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <span>
#include <vector>

#include "owl/render/RenderTypes.hpp"

namespace owl::desktop
{
class Viewport3DWindow
{
public:
    Viewport3DWindow() = default;
    Viewport3DWindow(const Viewport3DWindow&) = delete;
    Viewport3DWindow& operator=(const Viewport3DWindow&) = delete;
    Viewport3DWindow(Viewport3DWindow&&) = delete;
    Viewport3DWindow& operator=(Viewport3DWindow&&) = delete;
    ~Viewport3DWindow();

    bool Initialize(HWND parentHwnd, HDC parentDc, HGLRC parentGlrc);
    void Shutdown();

    void DrawImGuiPanel(std::span<const owl::render::Polyline3D> polylines);

    static LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void UpdatePlacementFromImGui();
    void RenderFrame();
    void RenderSceneFixedFunction();

    void ResetCamera();
    void FitCameraToContent();
    void UpdateBounds();

    struct Bounds
    {
        owl::render::Vec3 min {};
        owl::render::Vec3 max {};
        bool valid {false};
    };

    HWND parentHwnd_ {};
    HDC parentDc_ {};
    HGLRC parentGlrc_ {};

    HWND childHwnd_ {};
    HDC childDc_ {};
    HGLRC childGlrc_ {};
    std::uint32_t width_ {1};
    std::uint32_t height_ {1};
    bool visible_ {false};
    bool initialized_ {false};

    bool mouseDragging_ {false};
    POINT lastMousePos_ {};

    float yawDeg_ {45.0f};
    float pitchDeg_ {30.0f};
    float distance_ {120.0f};
    bool autoFit_ {true};

    owl::render::Vec3 target_ {};
    Bounds bounds_ {};
    std::vector<owl::render::Polyline3D> polylines_ {};

    bool placementDirty_ {false};
    bool wantRender_ {false};
    bool desiredVisible_ {false};
    RECT desiredRect_ {};
};
}

#endif
