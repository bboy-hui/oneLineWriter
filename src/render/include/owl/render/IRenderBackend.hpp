#pragma once

#include <memory>
#include <span>

#include "owl/render/RenderTypes.hpp"

namespace owl::render
{
struct RenderInitOptions
{
    BackendType backend {BackendType::Vulkan};
    void* nativeWindowHandle {};
    std::uint32_t viewportWidth {1280};
    std::uint32_t viewportHeight {720};
};

class IRenderBackend
{
public:
    virtual ~IRenderBackend() = default;
    virtual bool Initialize(const RenderInitOptions& options) = 0;
    virtual void Resize(std::uint32_t width, std::uint32_t height) = 0;
    virtual void DrawFrame(const CameraState& camera, std::span<const Polyline3D> polylines) = 0;
    [[nodiscard]] virtual const RenderFrameStats& LastFrameStats() const noexcept = 0;
    virtual void Shutdown() = 0;
};

using RenderBackendPtr = std::unique_ptr<IRenderBackend>;
}
