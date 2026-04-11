#include "owl/render/RenderBackendFactory.hpp"

namespace owl::render
{
RenderBackendPtr CreateOpenGLRenderBackend();
RenderBackendPtr CreateVulkanRenderBackend();

RenderBackendPtr CreateRenderBackend(BackendType backendType)
{
    switch (backendType)
    {
    case BackendType::OpenGL:
#if defined(OWL_ENABLE_OPENGL_BACKEND)
        return CreateOpenGLRenderBackend();
#else
        break;
#endif
    case BackendType::Vulkan:
#if defined(OWL_ENABLE_VULKAN_BACKEND)
        return CreateVulkanRenderBackend();
#else
        break;
#endif
    }

#if defined(OWL_ENABLE_OPENGL_BACKEND)
    return CreateOpenGLRenderBackend();
#elif defined(OWL_ENABLE_VULKAN_BACKEND)
    return CreateVulkanRenderBackend();
#else
    return nullptr;
#endif
}
}
