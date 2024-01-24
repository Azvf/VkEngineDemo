#include "SwapChain.h"

#include <iostream>
#include <set>

#include "runtime/core/base/exception.h"

#include "Texture.h"
#include "VKContext.h"
#include "VkUtil.h"

namespace Chandelier
{
    SwapChain::~SwapChain() { destroy(); }

    void SwapChain::Initialize(std::shared_ptr<VKContext> context, uint32_t width, uint32_t height)
    {
        m_context = context;

        createSwapChain(width, height);
        createRenderPass();
        createDepthBuffer();
    }

    void SwapChain::Free() { destroy(); }

    void SwapChain::createSwapChain(uint32_t width, uint32_t height)
    {
        const auto& phy_device = m_context->getPhysicalDevice();
        const auto& device     = m_context->getDevice();
        const auto& surface    = m_context->getSurface();

        // create SwapChain
        SwapChainSupportDetails swapChainSupport = m_context->QuerySwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR   presentMode   = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D         extent = chooseSwapExtent(swapChainSupport.capabilities, width, height);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo {};
        createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount    = imageCount;
        createInfo.imageFormat      = surfaceFormat.format;
        createInfo.imageColorSpace  = surfaceFormat.colorSpace;
        createInfo.imageExtent      = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices              = m_context->FindQueueFamilies();
        uint32_t           queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                                   indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode    = presentMode;
        createInfo.clipped        = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VULKAN_API_CALL(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapChain));

        vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, m_swapChainImages.data());

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent      = extent;

        // create image view
        m_swapChainImageViews.resize(m_swapChainImages.size());
        for (size_t i = 0; i < m_swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo {};
            createInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image    = m_swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format   = m_swapChainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel   = 0;
            createInfo.subresourceRange.levelCount     = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount     = 1;

            VULKAN_API_CALL(
                vkCreateImageView(device, &createInfo, nullptr, &m_swapChainImageViews[i]));
        }
    }

    void SwapChain::createRenderPass()
    {
        VkAttachmentDescription colorAttachment {};
        colorAttachment.format  = m_swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments    = &colorAttachmentRef;

        VkAttachmentDescription depthAttachment {};
        depthAttachment.format         = findDepthFormat();
        depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo                 renderPassInfo {};
        renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;

        VkSubpassDependency dependency {};
        dependency.srcSubpass   = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass   = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        VULKAN_API_CALL(
            vkCreateRenderPass(m_context->getDevice(), &renderPassInfo, nullptr, &m_renderPass));
    }

    void SwapChain::createDepthBuffer()
    {
        VkFormat depthFormat = findDepthFormat();

        m_depth_image = std::make_shared<Texture>();
        m_depth_image->InitTex2D(m_context,
                                 m_swapChainExtent.width,
                                 m_swapChainExtent.height,
                                 1,
                                 1,
                                 depthFormat,
                                 VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    void SwapChain::recreate(uint32_t width, uint32_t height)
    {
        vkDeviceWaitIdle(m_context->getDevice());
        destroy();

        createSwapChain(width, height);
        createRenderPass();
        createDepthBuffer();
    }

    void SwapChain::destroy()
    {
        const auto& device = m_context->getDevice();

        vkDestroyRenderPass(device, m_renderPass, nullptr);

        for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
        {
            vkDestroyImageView(device, m_swapChainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(device, m_swapChain, nullptr);
    }

    VkFormat SwapChain::findSupportedFormat(const std::vector<VkFormat>& candidates,
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

    VkFormat SwapChain::findDepthFormat()
    {
        return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkExtent2D SwapChain::getExtent() const { return m_swapChainExtent; }

    VkFormat SwapChain::getImageFormat() const { return m_swapChainImageFormat; }

    SwapChain::operator VkSwapchainKHR() const { return m_swapChain; }

    VkImage SwapChain::getImage(size_t index) const { return m_swapChainImages[index]; }

    VkImageView SwapChain::getImageView(size_t index) const { return m_swapChainImageViews[index]; }

    size_t SwapChain::getImageCount() const { return m_swapChainImageViews.size(); }

    VkRenderPass SwapChain::getRenderPass() const { return m_renderPass; }

    VkSwapchainKHR SwapChain::getSwapchain() const { return m_swapChain; }

} // namespace Chandelier
