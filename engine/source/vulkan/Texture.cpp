#include "Texture.h"

#include <cassert>
#include <iostream>

#include "runtime/core/base/exception.h"

#include "Buffer.h"
#include "CommandBuffers.h"
#include "VkContext.h"
#include "VkUtil.h"

namespace Chandelier
{
    Texture::~Texture() {}

    void Texture::EnsureImageView()
    {
        if (m_view_dirty)
        {
            ImageViewUpdate();
            m_view_dirty = false;
        }
    }

    void Texture::ImageViewUpdate()
    {
        const auto& device = m_context->getDevice();

        VkImageView view {};

        const VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;

        VkImageViewCreateInfo view_info {};
        view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image                           = m_image;
        view_info.viewType                        = m_view_type;
        view_info.format                          = m_format;
        view_info.subresourceRange.aspectMask     = aspect_flags;
        view_info.subresourceRange.baseMipLevel   = 0;
        view_info.subresourceRange.levelCount     = m_mip_levels;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount     = m_layers;

        VULKAN_API_CALL(vkCreateImageView(device, &view_info, nullptr, &view));

        m_view.emplace(view);
    }

    void Texture::EnsureLayout(VkImageLayout        current_layout,
                               VkImageLayout        requested_layout,
                               VkPipelineStageFlags src_stage,
                               VkAccessFlags        src_access,
                               VkPipelineStageFlags dst_stage,
                               VkAccessFlags        dst_access)
    {
        assert(m_image != VK_NULL_HANDLE);

        // fixme
        VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;

        VkImageMemoryBarrier barrier {};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = current_layout;
        barrier.newLayout                       = requested_layout;
        barrier.srcAccessMask                   = src_access;
        barrier.dstAccessMask                   = dst_access;
        barrier.image                           = m_image;
        barrier.subresourceRange.aspectMask     = aspect_flags;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = m_mip_levels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = m_layers;

        m_context->GetCommandBuffers().IssuePipelineBarrier(
            src_stage, dst_stage, std::vector<VkImageMemoryBarrier> {barrier});
    }

    void Texture::TransferLayout(VkImageLayout        requested_layout,
                                 VkPipelineStageFlags src_stage,
                                 VkAccessFlags        src_access,
                                 VkPipelineStageFlags dst_stage,
                                 VkAccessFlags        dst_access)
    {
        if (m_layout == requested_layout)
            return;

        EnsureLayout(m_layout, requested_layout, src_stage, src_access, dst_stage, dst_access);

        m_layout = requested_layout;
    }

    void Texture::InternalInit()
    {
        const auto& device = m_context->getDevice();
        assert(m_image == VK_NULL_HANDLE);
        // assert(!is_texture_view());

        VkImageCreateInfo image_info = {};
        image_info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.flags             = 0;
        image_info.imageType         = m_image_type;
        image_info.extent.width      = m_height;
        image_info.extent.height     = m_width;
        image_info.extent.depth      = m_depth;
        image_info.mipLevels         = std::max((int)m_mip_levels, 1);
        image_info.arrayLayers       = 1;
        image_info.format            = m_format;
        /* Some platforms (NVIDIA) requires that attached textures are always tiled optimal.
         *
         * As image data are always accessed via an staging buffer we can enable optimal tiling for
         * all texture. Tilings based on actual usages should be done in `VKFramebuffer`.
         */
        image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.usage         = m_usage;
        image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
        image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

        VULKAN_API_CALL(vkCreateImage(device, &image_info, nullptr, &m_image));

        VkMemoryRequirements mem_requirements;
        vkGetImageMemoryRequirements(device, m_image, &mem_requirements);

        VkMemoryAllocateInfo alloc_info {};
        alloc_info.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex =
            m_context->FindMemoryType(mem_requirements.memoryTypeBits, m_mem_props);

        VULKAN_API_CALL(vkAllocateMemory(device, &alloc_info, nullptr, &m_deviceMemory));
        VULKAN_API_CALL(vkBindImageMemory(device, m_image, m_deviceMemory, 0));

        m_view_type = (m_image_type == VK_IMAGE_TYPE_2D) ? VK_IMAGE_VIEW_TYPE_2D :
                                                           VK_IMAGE_VIEW_TYPE_2D_ARRAY;

        VkImageAspectFlags aspect_flags = ConvertToAspectFlags(m_usage);

        VkImageViewCreateInfo view_info {};
        view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image                           = m_image;
        view_info.viewType                        = m_view_type;
        view_info.format                          = m_format;
        view_info.subresourceRange.aspectMask     = aspect_flags;
        view_info.subresourceRange.baseMipLevel   = 0;
        view_info.subresourceRange.levelCount     = m_mip_levels;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount     = m_layers;

        VULKAN_API_CALL(vkCreateImageView(device, &view_info, nullptr, &m_view.value()));
    }

    void Texture::InitTex2D(std::shared_ptr<VKContext> context,
                            int                        width,
                            int                        height,
                            int                        layers,
                            int                        mip_len,
                            VkFormat                   format,
                            VkImageUsageFlags          usage,
                            VkMemoryPropertyFlags      mem_props)
    {
        m_width  = width;
        m_height = height;
        m_depth  = layers;

        int mip_len_max = 1 + floorf(log2f(std::max(width, height)));
        m_mip_levels    = std::min(mip_len, mip_len_max);
        m_format        = format;
        m_image_type    = VK_IMAGE_TYPE_2D;
        m_layout        = VK_IMAGE_LAYOUT_UNDEFINED;
        m_usage         = usage;
        m_mem_props     = mem_props;

        InternalInit();
    }

    void Texture::InitTex3D(std::shared_ptr<VKContext> context,
                            int                        width,
                            int                        height,
                            int                        depth,
                            int                        mip_len,
                            VkFormat                   format,
                            VkImageUsageFlags          usage)
    {
        assert(0);
        return;
    }

    void Texture::InitCubeMap(std::shared_ptr<VKContext> context,
                              int                        width,
                              int                        layers,
                              int                        mip_len,
                              VkFormat                   format,
                              VkImageUsageFlags          usage)
    {
        assert(0);
        return;
    }

    VkImageView Texture::getView()
    {
        EnsureImageView();
        return m_view.value();
    }

    VkImageAspectFlags Texture::ConvertToAspectFlags(VkImageUsageFlags usage)
    {
        VkImageAspectFlags aspect_flags;
        if (m_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        {
            aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        else if (m_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        else
        {
            assert(0);
        }
        return aspect_flags;
    }

    VkImage Texture::getImage() const { return m_image; }

    VkImageType Texture::getType() const { return m_image_type; }

    VkFormat Texture::getFormat() const { return m_format; }

    uint32_t Texture::getWidth() const { return m_width; }

    uint32_t Texture::getHeight() const { return m_height; }

    uint32_t Texture::getDepth() const { return m_depth; }

    uint32_t Texture::getLevels() const { return m_mip_levels; }

    uint32_t Texture::getLayers() const { return m_layers; }

    VkImageLayout Texture::getLayout() const { return m_layout; }

} // namespace Chandelier