#pragma once

#include "owl/render/IRenderBackend.hpp"

namespace owl::render
{
RenderBackendPtr CreateRenderBackend(BackendType backendType);
}
