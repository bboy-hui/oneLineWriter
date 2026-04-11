#include "owl/render/RenderService.hpp"

#include <utility>

namespace owl::render
{
RenderService::RenderService(RenderBackendPtr backend)
    : backend_(std::move(backend))
{
}

bool RenderService::Initialize(const RenderInitOptions& options)
{
    if (!backend_)
    {
        return false;
    }
    return backend_->Initialize(options);
}

void RenderService::Draw(const CameraState& camera, std::span<const Polyline3D> polylines)
{
    if (backend_)
    {
        backend_->DrawFrame(camera, polylines);
    }
}

RenderFrameStats RenderService::LastFrameStats() const
{
    if (!backend_)
    {
        return {};
    }
    return backend_->LastFrameStats();
}

void RenderService::Shutdown()
{
    if (backend_)
    {
        backend_->Shutdown();
    }
}
}
