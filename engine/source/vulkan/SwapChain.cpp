#include "SwapChain.h"

#include <iostream>
#include <set>

#include "runtime/core/base/exception.h"

#include "UI/window_system.h"
#include "Texture.h"
#include "VKContext.h"
#include "TimelineSemaphore.h"
#include "VkUtil.h"

namespace Chandelier
{
    SwapChain::~SwapChain() { Destroy(); }

    void SwapChain::Initialize(std::shared_ptr<VKContext> context, std::shared_ptr<WindowSystem> window_system)
    {
        m_context = context;
        m_window_system = window_system;
        CreateSwapChain();
    }

    void SwapChain::UnInit() { Destroy(); }

    VkPresentModeKHR SwapChain::SelectPresentMode()
    {
        const auto& phy_device = m_context->getPhysicalDevice();
        const auto& surface    = m_context->getSurface();

        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

        uint32_t present_mode_count;
        VULKAN_API_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(phy_device, surface, &present_mode_count, nullptr));
        assert(present_mode_count > 0);

        std::vector<VkPresentModeKHR> present_mode_vec(present_mode_count);
        VULKAN_API_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(
            phy_device, surface, &present_mode_count, present_mode_vec.data()));

        auto found_present_mode = [present_mode_vec](VkPresentModeKHR mode) -> bool {
            return (std::find(present_mode_vec.begin(), present_mode_vec.end(), mode) != present_mode_vec.end());
        };

        bool mailbox_mode_found   = found_present_mode(VK_PRESENT_MODE_MAILBOX_KHR);
        bool immediate_mode_found = found_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
        bool FIFO_mode_found      = found_present_mode(VK_PRESENT_MODE_FIFO_KHR);

        if (immediate_mode_found && !m_lock_to_VSync)
        {
            present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
        else if (mailbox_mode_found)
        {
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
        else if (FIFO_mode_found)
        {
            present_mode = VK_PRESENT_MODE_FIFO_KHR;
        }

        return present_mode;
    }

    VkSurfaceFormatKHR SwapChain::SelectSurfaceFormat()
    {
        const auto& phy_device = m_context->getPhysicalDevice();
        const auto& surface    = m_context->getSurface();

        uint32_t format_count;
        VULKAN_API_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, surface, &format_count, nullptr));
        assert(format_count);

        std::vector<VkSurfaceFormatKHR> formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, surface, &format_count, formats.data());
        
        for (const auto& surf_format : formats)
        {
            /**
             * @todo: VK_FORMAT_B8G8R8A8_UNORM will require us to manually do gamma correction in frag shader
             */
            if (surf_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                surf_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return surf_format;
            }
        }

        return formats.front();
    }

    VkExtent2D SwapChain::SelectExtent(const VkSurfaceCapabilitiesKHR& surf_caps, uint32_t width, uint32_t height)
    {
        const auto& phy_device = m_context->getPhysicalDevice();
        const auto& surface    = m_context->getSurface();

        if (width == 0 || height == 0)
        {
            assert(0);
            return VkExtent2D {width, height};
        }
        
        VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        actual_extent.width =
            std::clamp(actual_extent.width, surf_caps.minImageExtent.width, surf_caps.maxImageExtent.width);
        actual_extent.height =
            std::clamp(actual_extent.height, surf_caps.minImageExtent.height, surf_caps.maxImageExtent.height);

        return actual_extent;
    }

    void SwapChain::CreateSwapChain()
    {
        Vector2i size = m_window_system->GetFramebufferSize();

        if (size.x == 0 && size.y == 0)
        {
            assert(0);
            return;
        }

        const auto& phy_device = m_context->getPhysicalDevice();
        const auto& device     = m_context->getDevice();
        const auto& surface    = m_context->getSurface();

        VkSurfaceCapabilitiesKHR surf_caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy_device, surface, &surf_caps);

        VkSurfaceFormatKHR surf_format = this->SelectSurfaceFormat();
        VkExtent2D         extent      = this->SelectExtent(surf_caps, size.x, size.y);

        uint32_t imageCount = surf_caps.minImageCount + 1;
        if (surf_caps.maxImageCount > 0 && imageCount > surf_caps.maxImageCount)
        {
            imageCount = surf_caps.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo {};
        createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount    = imageCount;
        createInfo.imageFormat      = surf_format.format;
        createInfo.imageColorSpace  = surf_format.colorSpace;
        createInfo.imageExtent      = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        createInfo.preTransform     = surf_caps.currentTransform;
        createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode      = SelectPresentMode();
        createInfo.clipped          = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VULKAN_API_CALL(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_handle));

        VULKAN_API_CALL(vkGetSwapchainImagesKHR(device, m_handle, &imageCount, nullptr));
        m_swapChainImages.resize(imageCount);
        VULKAN_API_CALL(vkGetSwapchainImagesKHR(device, m_handle, &imageCount, m_swapChainImages.data()));

        for (int i = 0; i < m_swapChainImages.size(); i++)
        {
            TransferSwapchainImage(i, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        }

        // m_semaphore = std::make_unique<TimelineSemaphore>();
        // m_semaphore->Init(m_context);

        VkFenceCreateInfo fence_info = {};
        fence_info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VULKAN_API_CALL(vkCreateFence(m_context->getDevice(), &fence_info, nullptr, &m_fence));

        m_swapChainImageFormat = surf_format.format;
        m_swapChainExtent      = extent;

        m_swapChainImageViews.resize(m_swapChainImages.size());
        for (size_t i = 0; i < m_swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image                 = m_swapChainImages[i];
            createInfo.viewType              = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format                = m_swapChainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel   = 0;
            createInfo.subresourceRange.levelCount     = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount     = 1;

            VULKAN_API_CALL(vkCreateImageView(device, &createInfo, nullptr, &m_swapChainImageViews[i]));
        }
    }

    void SwapChain::Recreate()
    {
        VULKAN_API_CALL(vkDeviceWaitIdle(m_context->getDevice()));
        Destroy();

        CreateSwapChain();
    }

    void SwapChain::Destroy()
    {
        const auto& device = m_context->getDevice();

        // m_semaphore = nullptr;
        // m_last_signal_value = {};
        vkDestroyFence(m_context->getDevice(), m_fence, nullptr);
        m_fence = VK_NULL_HANDLE;
        
        for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
        {
            vkDestroyImageView(device, m_swapChainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(device, m_handle, nullptr);
    }

    void SwapChain::AcquireImage(bool& resized) {
        const auto& device = m_context->getDevice();

        // todo: optimize performance
        VkResult result = vkAcquireNextImageKHR(
            device, m_handle, UINT64_MAX, VK_NULL_HANDLE, m_fence, &m_render_image_index);
        // m_semaphore->Wait(m_semaphore->GetValue());
        // m_semaphore->IncreaseValue();
        VULKAN_API_CALL(vkWaitForFences(device, 1, &m_fence, VK_TRUE, UINT64_MAX));
        VULKAN_API_CALL(vkResetFences(device, 1, &m_fence));

        if (result == VK_SUCCESS)
        {
            // do nothing
            resized = false;
        }
        else if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            Recreate();
            resized = true;
            return;
        }
        else if (result != VK_SUBOPTIMAL_KHR)
        {
            assert(0);
            resized = false;
            throw std::runtime_error("swapchain error!");
            return;
        }
    }

    void SwapChain::SwapBuffer(bool& resized) {
        static size_t swap_count = 0;

        VkPresentInfoKHR present_info = {};
        present_info.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // todo: optimize performance later, for now all the draw calls block on cpu till render finished 
        present_info.waitSemaphoreCount = 0;
        present_info.pWaitSemaphores    = nullptr;

        present_info.swapchainCount = 1;
        present_info.pSwapchains    = &m_handle;
        present_info.pImageIndices  = &m_render_image_index;
        present_info.pResults       = nullptr;

        VkResult result = vkQueuePresentKHR(m_context->getPresentQueue(), &present_info);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR )
        {
            Recreate();
            resized = true;
            return;
        }
        else if (result != VK_SUCCESS)
        {
            assert(0);
            resized = false;
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_context->IncFrameIndex();
        swap_count++;
    }

    void SwapChain::TransferSwapchainImage(size_t index, VkImageLayout requested_layout)
    {
        std::vector<VkImageMemoryBarrier> barriers;

        VkImageMemoryBarrier barrier        = {};
        barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                   = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout                   = requested_layout;
        barrier.image                       = m_swapChainImages[index];
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        barriers.push_back(barrier);

        auto& command_manager = m_context->GetCommandManager();
        command_manager.PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, barriers);
        // command_manager.Submit();
    }

    VkFormat SwapChain::FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                            VkImageTiling                tiling,
                                            VkFormatFeatureFlags         features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_context->getPhysicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR &&
                (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                     (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat SwapChain::FindDepthFormat()
    {
        return FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkExtent2D SwapChain::getExtent() const { return m_swapChainExtent; }

    VkFormat SwapChain::getImageFormat() const { return m_swapChainImageFormat; }

    SwapChain::operator VkSwapchainKHR() const { return m_handle; }

    VkImage SwapChain::getImage(size_t index) const { return m_swapChainImages[index]; }

    VkImageView SwapChain::getImageView(size_t index) const { return m_swapChainImageViews[index]; }

    size_t SwapChain::getImageCount() const { return m_swapChainImageViews.size(); }

    VkSwapchainKHR SwapChain::getSwapchain() const { return m_handle; }

} // namespace Chandelier
