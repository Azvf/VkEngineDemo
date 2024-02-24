#include "Texture.h"

#include <cassert>
#include <iostream>

#include "runtime/core/base/exception.h"

#include "Buffer.h"
#include "CommandBuffers.h"
#include "VkContext.h"
#include "Buffer.h"
#include "VkUtil.h"

namespace Chandelier
{
    size_t TextureFormatToByteSize(VkFormat format) { 
        size_t byte_size = {};
        switch (format)
        {
            case VK_FORMAT_R8G8B8A8_UNORM:
                byte_size = 4;
                break;
            case VK_FORMAT_R8G8B8A8_SRGB:
                byte_size = 4;
                break;
            case VK_FORMAT_R32G32_SFLOAT:
                byte_size = 8;
                break;
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                byte_size = 8;
                break;
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                byte_size = 16;
                break;
            default:
                assert(0);
                break;
        }
        return byte_size;
    }

    Texture::~Texture()
    {
        if (m_buffer && m_buffer->IsMapped())
        {
            m_buffer->unmap();
        }
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

        VkImageMemoryBarrier barrier = {};
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

        m_context->GetCommandManager().IssuePipelineBarrier(
            src_stage, dst_stage, std::vector<VkImageMemoryBarrier> {barrier});
    }

    void Texture::TransferLayout(VkImageLayout        requested_layout,
                                 VkPipelineStageFlags src_stage,
                                 VkAccessFlags        src_access,
                                 VkPipelineStageFlags dst_stage,
                                 VkAccessFlags        dst_access)
    {
        // assert(m_layout == requested_layout);
        if (m_layout == requested_layout)
            return;

        EnsureLayout(m_layout, requested_layout, src_stage, src_access, dst_stage, dst_access);

        m_layout = requested_layout;
    }

    void Texture::TransferLayout(VkImageLayout        current_layout,
                                 VkImageLayout        requested_layout,
                                 VkPipelineStageFlags src_stage,
                                 VkAccessFlags        src_access,
                                 VkPipelineStageFlags dst_stage,
                                 VkAccessFlags        dst_access)
    {
        EnsureLayout(current_layout, requested_layout, src_stage, src_access, dst_stage, dst_access);
        m_layout = requested_layout;
    }

    void Texture::InternalInit()
    {
        const auto& device = m_context->getDevice();
        assert(m_image == VK_NULL_HANDLE);
        // assert(!is_texture_view());

        VkImageCreateInfo image_info = {};
        image_info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.flags             = m_cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
        image_info.imageType         = m_image_type;
        image_info.extent.width      = m_width;
        image_info.extent.height     = m_height;
        image_info.extent.depth      = 1;
        image_info.mipLevels         = std::max((int)m_mip_levels, 1);
        image_info.arrayLayers       = m_layers;
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

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex =
            m_context->FindMemoryType(mem_requirements.memoryTypeBits, m_mem_props);

        VULKAN_API_CALL(vkAllocateMemory(device, &alloc_info, nullptr, &m_device_memory));
        VULKAN_API_CALL(vkBindImageMemory(device, m_image, m_device_memory, 0));

        m_view_type = (m_cube) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;

        VkImageAspectFlags aspect_flags = ConvertToAspectFlags(m_usage);

        VkImageView           view;
        VkImageViewCreateInfo view_info = {};
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

    void Texture::InitTex2D(std::shared_ptr<VKContext> context,
                            int                        width,
                            int                        height,
                            int                        layers,
                            int                        mip_len,
                            VkFormat                   format,
                            VkImageUsageFlags          usage,
                            VkMemoryPropertyFlags      mem_props)
    {
        m_context = context;
        m_cube    = false;

        m_width  = width;
        m_height = height;
        m_layers  = layers;

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
    }

    void Texture::InitCubeMap(std::shared_ptr<VKContext> context,
                              int                        width,
                              int                        layers,
                              int                        mip_len,
                              VkFormat                   format,
                              VkImageUsageFlags          usage)
    {
        assert(0 && "wip, not tested");
        m_context = context;
        m_cube    = true;

        m_width  = width;
        m_height = width;
        m_layers = std::max(layers, 1) * 6;

        int mip_len_max = 1 + floorf(log2f(std::max(m_width, m_height)));
        m_mip_levels    = std::min(mip_len, mip_len_max);
        m_format        = format;
        m_image_type    = VK_IMAGE_TYPE_2D;
        m_layout        = VK_IMAGE_LAYOUT_UNDEFINED;
        m_usage         = usage;
        m_mem_props     = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        
        InternalInit();
    }

    void Texture::InitCubeMap(std::shared_ptr<VKContext>              context,
                              std::array<std::shared_ptr<Texture>, 6> faces,
                              int                                     mip_len,
                              VkFormat                                format)
    {
        m_context = context;
        m_cube    = true;

        assert(faces.front() && faces.back());
        
        m_width = faces.front()->getWidth();
        m_height = faces.front()->getHeight();
        
        int mip_len_max = 1 + floorf(log2f(std::max(m_width, m_height)));
        m_mip_levels    = std::min(mip_len, mip_len_max);
        m_format        = format;
        m_image_type    = VK_IMAGE_TYPE_2D;
        m_layers        = 6;
        m_usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        
        InternalInit();
    }

    void Texture::Sync(const uint8_t* data)
    {
        auto         staging_buffer   = std::make_unique<Buffer>();
        size_t       image_layer_size = GetLayerByteSize();
        assert(!m_cube && "not sure if the size is accurate when loading one single cube file");
        VkDeviceSize data_size        = m_cube ? image_layer_size * 6 : image_layer_size;
        staging_buffer->Allocate(m_context,
                                 data_size,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        staging_buffer->map();
        staging_buffer->Update(reinterpret_cast<const uint8_t*>(data), data_size);
        staging_buffer->Flush();
        staging_buffer->unmap();

        m_context->CopyBufferToTexture(staging_buffer.get(), this);

        m_context->GetCommandManager().Submit();
    }

    void Texture::Sync(std::vector<std::shared_ptr<Texture>> textures) {
        auto staging_buffer = std::make_unique<Buffer>();

        size_t       image_layer_size = GetLayerByteSize();
        VkDeviceSize data_size        = m_cube ? image_layer_size * 6 : image_layer_size;
        staging_buffer->Allocate(m_context,
                                 data_size,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        staging_buffer->map();

        size_t offset = {};
        for (auto& texture : textures)
        {
            uint8_t* data = texture->Data();
            size_t   size = texture->GetLayerByteSize();

            staging_buffer->Update(data, size, 0, offset);
            
            offset += size;
        }

        staging_buffer->Flush();
        staging_buffer->unmap();

        m_context->CopyBufferToTexture(staging_buffer.get(), this);

        m_context->GetCommandManager().Submit();
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
        else if (m_usage & VK_IMAGE_USAGE_SAMPLED_BIT)
        {
            aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        else
        {
            assert(0);
        }
        return aspect_flags;
    }

    size_t Texture::GetLayerByteSize()
    {
        /**
         * @info: not taking mips into consideration
         */
        size_t       pixel_byte_size = TextureFormatToByteSize(m_format);
        VkDeviceSize data_size       = m_width * m_height * pixel_byte_size;
        return data_size;
    }

    uint8_t* Texture::Data()
    {
        if (!m_buffer)
        {
            m_buffer = std::make_shared<Buffer>();

            VkDeviceSize data_size = GetLayerByteSize() * m_layers;
            m_buffer->Allocate(m_context,
                               data_size,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               VK_DESCRIPTOR_TYPE_MAX_ENUM);
        }

        uint8_t* data = nullptr;
        
        /**
         * @todo: caching, just return the mapped data if texture is not updated 
         */
        if (!m_buffer->IsMapped())
        {
            m_context->CopyTextureToBuffer(this, m_buffer.get());
        }
        
        /**
         * @info: blocking on cpu so we don't get empty data on memory 
         */
        m_context->GetCommandManager().Submit();
        
        data = m_buffer->map();

        return data;
    }

    VkImage Texture::getImage() const { return m_image; }

    VkImageType Texture::getType() const { return m_image_type; }

    VkFormat Texture::getFormat() const { return m_format; }

    uint32_t Texture::getWidth() const { return m_width; }

    uint32_t Texture::getHeight() const { return m_height; }

    uint32_t Texture::getLevels() const { return m_mip_levels; }

    uint32_t Texture::getLayers() const { return m_layers; }

    VkImageLayout Texture::getLayout() const { return m_layout; }

} // namespace Chandelier