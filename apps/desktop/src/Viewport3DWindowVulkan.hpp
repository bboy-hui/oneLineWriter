#pragma once

#if defined(_WIN32)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "owl/render/RenderTypes.hpp"

// Vulkan SDK is expected on Windows builds.
#include <vulkan/vulkan.h>

namespace owl::desktop
{
class Viewport3DWindowVulkan
{
public:
    Viewport3DWindowVulkan() = default;
    Viewport3DWindowVulkan(const Viewport3DWindowVulkan&) = delete;
    Viewport3DWindowVulkan& operator=(const Viewport3DWindowVulkan&) = delete;
    Viewport3DWindowVulkan(Viewport3DWindowVulkan&&) = delete;
    Viewport3DWindowVulkan& operator=(Viewport3DWindowVulkan&&) = delete;
    ~Viewport3DWindowVulkan();

    bool Initialize(HWND parentHwnd);
    void Shutdown();

    void DrawImGuiPanel(std::span<const owl::render::Polyline3D> polylines);

    [[nodiscard]] const std::string& LastError() const noexcept;

    static LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void UpdatePlacementFromImGui();
    bool EnsureChildWindow();
    void DestroyChildWindow();

    bool InitializeVulkan();
    void ShutdownVulkan();
    bool CreateOrResizeSwapchain();
    void DestroySwapchain();
    bool CreateDepthResources();
    void DestroyDepthResources();
    bool CreateRenderPass();
    void DestroyRenderPass();
    bool CreatePipeline();
    void DestroyPipeline();
    bool CreateFrameResources();
    void DestroyFrameResources();
    bool RenderFrame();
    void RequestSwapchainRebuild();

    void UpdateBounds();
    void FitCameraToContent();

    struct Bounds
    {
        owl::render::Vec3 min {};
        owl::render::Vec3 max {};
        bool valid {false};
    };

    HWND parentHwnd_ {};
    HWND childHwnd_ {};
    std::uint32_t width_ {1};
    std::uint32_t height_ {1};
    bool visible_ {false};
    bool initialized_ {false};

    bool desiredVisible_ {false};
    bool placementDirty_ {false};
    RECT desiredRect_ {};

    bool swapchainDirty_ {false};
    bool autoFit_ {true};
    bool mouseDragging_ {false};
    POINT lastMousePos_ {};

    float yawDeg_ {45.0f};
    float pitchDeg_ {30.0f};
    float distance_ {120.0f};
    owl::render::Vec3 target_ {};
    Bounds bounds_ {};

    std::vector<owl::render::Polyline3D> polylines_ {};

    std::string lastError_;

    VkInstance instance_ {VK_NULL_HANDLE};
    VkSurfaceKHR surface_ {VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice_ {VK_NULL_HANDLE};
    VkDevice device_ {VK_NULL_HANDLE};
    VkQueue graphicsQueue_ {VK_NULL_HANDLE};
    std::uint32_t graphicsQueueFamily_ {0xFFFFFFFFu};

    VkSwapchainKHR swapchain_ {VK_NULL_HANDLE};
    VkFormat swapchainFormat_ {VK_FORMAT_UNDEFINED};
    VkRenderPass renderPass_ {VK_NULL_HANDLE};

    std::vector<VkImage> swapchainImages_ {};
    std::vector<VkImageView> swapchainImageViews_ {};
    std::vector<VkFramebuffer> framebuffers_ {};

    VkImage depthImage_ {VK_NULL_HANDLE};
    VkDeviceMemory depthImageMemory_ {VK_NULL_HANDLE};
    VkImageView depthImageView_ {VK_NULL_HANDLE};
    VkFormat depthFormat_ {VK_FORMAT_UNDEFINED};

    VkCommandPool commandPool_ {VK_NULL_HANDLE};
    std::vector<VkCommandBuffer> commandBuffers_ {};

    VkSemaphore imageAvailableSemaphore_ {VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore_ {VK_NULL_HANDLE};
    VkFence inFlightFence_ {VK_NULL_HANDLE};

    VkDescriptorSetLayout descriptorSetLayout_ {VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout_ {VK_NULL_HANDLE};
    VkPipeline pipeline_ {VK_NULL_HANDLE};
    VkDescriptorPool descriptorPool_ {VK_NULL_HANDLE};
    VkDescriptorSet descriptorSet_ {VK_NULL_HANDLE};

    VkBuffer uniformBuffer_ {VK_NULL_HANDLE};
    VkDeviceMemory uniformBufferMemory_ {VK_NULL_HANDLE};
    VkBuffer vertexBuffer_ {VK_NULL_HANDLE};
    VkDeviceMemory vertexBufferMemory_ {VK_NULL_HANDLE};
    std::size_t vertexBufferCapacityBytes_ {0};
    std::uint32_t lastVertexCount_ {0};
};
}

#endif
