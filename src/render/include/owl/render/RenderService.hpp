#pragma once

#include <memory>

#include "owl/render/IRenderBackend.hpp"

namespace owl::render
{
class RenderService
{
public:
    explicit RenderService(RenderBackendPtr backend);
    bool Initialize(const RenderInitOptions& options);
    void Draw(const CameraState& camera, std::span<const Polyline3D> polylines);
    [[nodiscard]] RenderFrameStats LastFrameStats() const;
    void Shutdown();

private:
    RenderBackendPtr backend_;
};
}
