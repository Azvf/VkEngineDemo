#include "Texture.h"

#include <iostream>
#include <cassert>

#include "runtime/core/base/exception.h"

#include "Buffer.h"
#include "VkContext.h"
#include "CommandBuffers.h"
#include "VkUtil.h"

namespace Chandelier
{
    Texture::~Texture() {}

    void Texture::Initialize(uint8_t* image_data, const TextureCreateInfo& info)
    {
        const auto& image = m_image->getImage();

        BufferCreateInfo   buffer_create_info;
        VkBufferCreateInfo vk_buffer_info {};
        VkDeviceSize       buffer_size = info.width * info.height * 4;
        vk_buffer_info.sType           = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vk_buffer_info.size            = buffer_size;
        vk_buffer_info.usage           = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        vk_buffer_info.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;

        buffer_create_info.vk_buffer_info = vk_buffer_info;
        buffer_create_info.size           = buffer_size;
        buffer_create_info.mem_flags      = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        auto buffer = Buffer::Create(buffer_create_info);

        void* data = buffer->map();
        memcpy(data, image_data, static_cast<size_t>(buffer_size));
        buffer->unmap();

        info.context->TransiteTextureLayout(shared_from_this(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        info.context->CopyBufferToTexture(buffer, shared_from_this());
        info.context->TransiteTextureLayout(shared_from_this(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void Texture::Initialize(VkImage vk_image, VkImageLayout layout, VkFormat format)
    {
        m_image  = vk_image;
        m_layout = layout;
        m_format = format;
    }

    std::shared_ptr<Texture> Texture::Create(uint8_t* image_data, const TextureCreateInfo& info)
    {
        auto tex = std::make_shared<Texture>();
        tex->Initialize(image_data, info);
        return tex;
    }

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
        view_info.subresourceRange.levelCount     = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount     = 1;

        VULKAN_API_CALL(vkCreateImageView(device, &view_info, nullptr, &view));

        m_view.emplace(view);
    }

    void Texture::EnsureLayout(VKContextPtr         context,
        Range                mipmap_range,
        VkImageLayout        current_layout,
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
        barrier.subresourceRange.baseMipLevel   = uint32_t(mipmap_range.start());
        barrier.subresourceRange.levelCount     = uint32_t(mipmap_range.size());
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
        
        m_context->GetCommandBuffers()->IssuePipelineBarrier(
            src_stage, dst_stage, std::vector<VkImageMemoryBarrier> {barrier});
    }

    void Texture::TransferLayout(VKContextPtr         context,
        VkImageLayout        requested_layout,
        VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VkAccessFlags        src_access = VK_ACCESS_MEMORY_WRITE_BIT,
        VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VkAccessFlags        dst_access = VK_ACCESS_MEMORY_READ_BIT)
    {
        if (m_layout == requested_layout)
            return;

        EnsureLayout(context,
                     Range {0, VK_REMAINING_MIP_LEVELS},
                     m_layout,
                     requested_layout,
                     src_stage,
                     src_access,
                     dst_stage,
                     dst_access);

        m_layout = requested_layout;
    }

    VkImageView Texture::getView()
    {
        EnsureImageView();
        return m_view.value();
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