#pragma once

#include <optional>

#include "VkCreateInfo.h"
#include "runtime/core/base/index_range.h"

namespace Chandelier
{
    class Buffer;
    
    using TexturePtr = std::shared_ptr<Texture>;

    class Texture 
    {
    public:
        Texture() = default;
        ~Texture();

        friend class VKContext;

        Texture(const Texture&)             = delete;
        Texture(const Texture&&)            = delete;
        Texture& operator=(const Texture&)  = delete;
        Texture& operator=(const Texture&&) = delete;

        VkImageView getView();

        VkImage       getImage() const;
        VkImageType   getType() const;
        VkFormat      getFormat() const;
        uint32_t      getWidth() const;
        uint32_t      getHeight() const;
        uint32_t      getLevels() const;
        uint32_t      getLayers() const;
        VkImageLayout getLayout() const;

        uint8_t* Data();
        size_t   GetLayerByteSize();

        void InitTex2D(std::shared_ptr<VKContext> context,
                       int                        width,
                       int                        height,
                       int                        layers,
                       int                        mip_len,
                       VkFormat                   format,
                       VkImageUsageFlags          usage,
                       VkMemoryPropertyFlags      mem_props);

        void InitTex3D(std::shared_ptr<VKContext> context,
                       int                        width,
                       int                        height,
                       int                        depth,
                       int                        mip_len,
                       VkFormat                   format,
                       VkImageUsageFlags          usage);

        void InitCubeMap(std::shared_ptr<VKContext> context,
                         int                        width,
                         int                        layers,
                         int                        mip_len,
                         VkFormat                   format,
                         VkImageUsageFlags          usage);
        
        void InitCubeMap(std::shared_ptr<VKContext>              context,
                         std::array<std::shared_ptr<Texture>, 6> faces,
                         int                                     mip_len,
                         VkFormat                                format);

        void TransferLayout(VkImageLayout        requested_layout,
                            VkPipelineStageFlags src_stage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VkAccessFlags        src_access = VK_ACCESS_MEMORY_WRITE_BIT,
                            VkPipelineStageFlags dst_stage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VkAccessFlags        dst_access = VK_ACCESS_MEMORY_READ_BIT);

        void TransferLayout(VkImageLayout        current_layout,
                            VkImageLayout        requested_layout,
                            VkPipelineStageFlags src_stage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VkAccessFlags        src_access = VK_ACCESS_MEMORY_WRITE_BIT,
                            VkPipelineStageFlags dst_stage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VkAccessFlags        dst_access = VK_ACCESS_MEMORY_READ_BIT);

        void Sync(const uint8_t* data);
        void Sync(std::vector<std::shared_ptr<Texture>> textures);

    private:
        void EnsureImageView();
        void ImageViewUpdate();

        void EnsureLayout(VkImageLayout        current_layout,
                          VkImageLayout        requested_layout,
                          VkPipelineStageFlags src_stage,
                          VkAccessFlags        src_access,
                          VkPipelineStageFlags dst_stage,
                          VkAccessFlags        dst_access);

        void InternalInit();

        VkImageAspectFlags ConvertToAspectFlags(VkImageUsageFlags usage);

    private:
        std::shared_ptr<VKContext> m_context;

        VkImage                    m_image;
        VkDeviceMemory             m_device_memory;
        std::optional<VkImageView> m_view;
        VkImageViewType            m_view_type;

        VkImageType           m_image_type;
        VkFormat              m_format;
        uint32_t              m_width;
        uint32_t              m_height;
        uint32_t              m_mip_levels;
        uint32_t              m_layers;
        /**
         * @warning: vulkan does not offer api for checking vkimage layout, thus the layout is just not accurate, 
         * thinking about deprecating the layout member
         */
        VkImageLayout         m_layout;
        VkImageUsageFlags     m_usage;
        VkMemoryPropertyFlags m_mem_props;

        /**
         * @todo: will using mapping be a performance hit, or just maintain the pointer got from stb loading? 
         */
        std::shared_ptr<Buffer> m_buffer;
        uint8_t*                data = nullptr;

        bool m_view_dirty = false;

        bool m_cube = false;
    };

    extern size_t TextureFormatToByteSize(VkFormat format);

} // namespace Chandelier