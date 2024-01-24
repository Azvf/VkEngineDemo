#pragma once

#include <vector>
#include <memory>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Chandelier
{
    class VKContext;
    class Texture;

    class SwapChain
    {
    public:
        SwapChain() = default;
        ~SwapChain();

        void Initialize(std::shared_ptr<VKContext> context, uint32_t width, uint32_t height);
        void Free();

        SwapChain(SwapChain&)                   = delete;
        SwapChain(SwapChain&&)                  = delete;
        SwapChain& operator=(const SwapChain&)  = delete;
        SwapChain& operator=(const SwapChain&&) = delete;

        VkExtent2D getExtent() const;
        VkFormat   getImageFormat() const;
        operator VkSwapchainKHR() const;
        VkImage        getImage(size_t index) const;
        VkImageView    getImageView(size_t index) const;
        size_t         getImageCount() const;
        VkRenderPass   getRenderPass() const;
        VkFramebuffer  getFramebuffer(uint32_t index) const;
        VkSwapchainKHR getSwapchain() const;

        void recreate(uint32_t width, uint32_t height);

    private:
        void createSwapChain(uint32_t width, uint32_t height);
        void createRenderPass();
        void createDepthBuffer();
        void destroy();

    private:
        VkFormat findDepthFormat();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags         features);

    private:
        std::shared_ptr<VKContext> m_context;

        VkSwapchainKHR m_swapChain;
        VkFormat       m_swapChainImageFormat;
        VkExtent2D     m_swapChainExtent;

        VkRenderPass m_renderPass;

        std::vector<VkImage>     m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        VkImage        m_depthImage;
        VkDeviceMemory m_depthImageMemory;
        VkImageView    m_depthImageView;

        std::shared_ptr<Texture> m_depth_image;
    };
} // namespace Chandelier