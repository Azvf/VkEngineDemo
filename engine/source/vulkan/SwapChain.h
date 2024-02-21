#pragma once

#include "VkCommon.h"
#include "TimelineSemaphore.h"

namespace Chandelier
{
    class VKContext;
    class Texture;
    class TimelineSemaphore;
    class WindowSystem;

    class SwapChain
    {
    public:
        SwapChain() = default;
        ~SwapChain();

        void Initialize(std::shared_ptr<VKContext> context, std::shared_ptr<WindowSystem> window_system);
        void UnInit();

        VkExtent2D getExtent() const;
        VkFormat   getImageFormat() const;
        operator VkSwapchainKHR() const;
        VkImage        getImage(size_t index) const;
        VkImageView    getImageView(size_t index) const;
        size_t         getImageCount() const;
        VkSwapchainKHR getSwapchain() const;

        void Recreate();
        
        void TransferSwapchainImage(size_t index, VkImageLayout requested_layout);
        
        void AcquireImage(bool& resized);
        void SwapBuffer(bool& resized);

    private:
        void CreateSwapChain();
        void Destroy();

    private:
        VkFormat FindDepthFormat();
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags         features);

    private:
        std::shared_ptr<VKContext>    m_context;
        std::shared_ptr<WindowSystem> m_window_system;

        VkSwapchainKHR m_handle;
        VkFormat       m_swapChainImageFormat;
        VkExtent2D     m_swapChainExtent;

        std::vector<VkImage>     m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        /** 
         * vkAcquireNextImageKHR():  VkSemaphore 0xcad092000000000d[] is not a VK_SEMAPHORE_TYPE_BINARY. 
         * The Vulkan spec states: semaphore must have a VkSemaphoreType of VK_SEMAPHORE_TYPE_BINARY 
         * (https://vulkan.lunarg.com/doc/view/1.3.275.0/windows/1.3-extensions/vkspec.html#VUID-vkAcquireNextImageKHR-semaphore-03265)
         * @info: vkAcquireNextImageKHR does not seem to support timeline sempahore, use VkFence for now
         */
        // std::unique_ptr<TimelineSemaphore> m_semaphore;
        // TimelineSemaphore::Value           m_last_signal_value;

        VkFence m_fence = VK_NULL_HANDLE;

        uint32_t m_render_image_index;
    };
} // namespace Chandelier