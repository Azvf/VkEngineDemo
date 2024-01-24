#pragma once

#include <optional>

#include "VkCreateInfo.h"
#include "runtime/core/base/index_range.h"

namespace Chandelier
{
    using TexturePtr = std::shared_ptr<Texture>;

    class Texture : std::enable_shared_from_this<Texture>
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
        uint32_t      getDepth() const;
        uint32_t      getLevels() const;
        uint32_t      getLayers() const;
        VkImageLayout getLayout() const;

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

        void TransferLayout(VkImageLayout        requested_layout,
                            VkPipelineStageFlags src_stage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VkAccessFlags        src_access = VK_ACCESS_MEMORY_WRITE_BIT,
                            VkPipelineStageFlags dst_stage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VkAccessFlags        dst_access = VK_ACCESS_MEMORY_READ_BIT);

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
        VkDeviceMemory             m_deviceMemory;
        std::optional<VkImageView> m_view;
        VkImageViewType            m_view_type;

        VkImageType           m_image_type;
        VkFormat              m_format;
        uint32_t              m_width;
        uint32_t              m_height;
        uint32_t              m_depth;
        uint32_t              m_mip_levels;
        uint32_t              m_layers;
        VkImageLayout         m_layout;
        VkImageUsageFlags     m_usage;
        VkMemoryPropertyFlags m_mem_props;

        bool m_view_dirty = false;
    };
} // namespace Chandelier