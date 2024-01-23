#pragma once

#include <optional>

#include "VkCreateInfo.h"
#include "runtime/core/base/index_range.h"

namespace Chandelier
{
    using TexturePtr = std::shared_ptr<Texture>;

    enum eImageViewUsage : uint8_t
    {
        /**
         * @brief: Image View will be used as a bindable shader resource.
         */
        ShaderBinding,
        /**
         * @brief: Image View will be used as an frame-buffer attachment.
         */
        Attachment,
    };

    class Texture : std::enable_shared_from_this<Texture>
    {
    public:
        Texture() = default;
        ~Texture();

        friend class VKContext;

        void                            Initialize(uint8_t* image_data, const TextureCreateInfo& info);
        void                            Initialize(VkImage vk_image, VkImageLayout layout, VkFormat format);
        static std::shared_ptr<Texture> Create(uint8_t* image_data, const TextureCreateInfo& info);

        Texture(const Texture&)             = delete;
        Texture(const Texture&&)            = delete;
        Texture& operator=(const Texture&)  = delete;
        Texture& operator=(const Texture&&) = delete;

        VkImageView   getView();
        VkImage       getImage() const;
        VkImageType   getType() const;
        VkFormat      getFormat() const;
        uint32_t      getWidth() const;
        uint32_t      getHeight() const;
        uint32_t      getDepth() const;
        uint32_t      getLevels() const;
        uint32_t      getLayers() const;
        VkImageLayout getLayout() const;

        // bool IsTexView() const;
        void InitTex2D(int width, int height, int layers, int mip_len, VkFormat format);
        void InitTex3D(int width, int height, int depth, int mip_len, VkFormat format);
        void InitCubeMap(int width, int layers, int mip_len, VkFormat format);

        void TransferLayout(std::shared_ptr<VKContext> context,
                            VkImageLayout              requested_layout,
                            VkPipelineStageFlags       src_stage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VkAccessFlags              src_access = VK_ACCESS_MEMORY_WRITE_BIT,
                            VkPipelineStageFlags       dst_stage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VkAccessFlags              dst_access = VK_ACCESS_MEMORY_READ_BIT);

    private:
        void EnsureImageView();
        void ImageViewUpdate();

        void EnsureLayout(std::shared_ptr<VKContext> context,
                          Range                      mipmap_range,
                          VkImageLayout              current_layout,
                          VkImageLayout              requested_layout,
                          VkPipelineStageFlags       src_stage,
                          VkAccessFlags              src_access,
                          VkPipelineStageFlags       dst_stage,
                          VkAccessFlags              dst_access);

    private:
        std::shared_ptr<VKContext> m_context;
        VkImage                    m_image;
        std::optional<VkImageView> m_view;
        VkDeviceMemory             m_deviceMemory;

        VkImageType     m_image_type;
        VkFormat        m_format;
        uint32_t        m_width;
        uint32_t        m_height;
        uint32_t        m_depth;
        uint32_t        m_mip_levels;
        uint32_t        m_layers;
        VkImageLayout   m_layout;
        VkImageViewType m_view_type;

        bool m_view_dirty = false;

        // eImageViewUsage m_view_usage = eImageViewUsage::ShaderBinding;
        // bool m_use_stencil = false;
    };
} // namespace Chandelier