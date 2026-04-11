#include "owl/render/IRenderBackend.hpp"
#include "RenderStatsUtils.hpp"

namespace owl::render
{
class VulkanRenderBackend final : public IRenderBackend
{
public:
    bool Initialize(const RenderInitOptions& options) override
    {
        initialized_ = true;
        width_ = options.viewportWidth;
        height_ = options.viewportHeight;
        return true;
    }

    void Resize(std::uint32_t width, std::uint32_t height) override
    {
        width_ = width;
        height_ = height;
    }

    void DrawFrame(const CameraState&, std::span<const Polyline3D> polylines) override
    {
        lastStats_ = ComputeFrameStats(polylines);
    }

    const RenderFrameStats& LastFrameStats() const noexcept override
    {
        return lastStats_;
    }

    void Shutdown() override
    {
        initialized_ = false;
    }

private:
    bool initialized_ {false};
    std::uint32_t width_ {0};
    std::uint32_t height_ {0};
    RenderFrameStats lastStats_ {};
};

RenderBackendPtr CreateVulkanRenderBackend()
{
    return std::make_unique<VulkanRenderBackend>();
}
}
