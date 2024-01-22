#pragma once

#include "VkCreateInfo.h"

namespace Chandelier
{
    class Image;
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

        VkImageView getView() const;
        VkImage     getImage() const;
        VkImageType getType() const;
        VkFormat    getFormat() const;
        uint32_t    getWidth() const;
        uint32_t    getHeight() const;
        uint32_t    getDepth() const;
        uint32_t    getLevels() const;
        uint32_t    getLayers() const;

        void                            Initialize(uint8_t* image_data, const TextureCreateInfo& info);
        static std::shared_ptr<Texture> Create(uint8_t* image_data, const TextureCreateInfo& info);

    private:
        // VkDevice       m_device;
        // VkImageView    m_view;
        // VkImage        m_image;
        // VkDeviceMemory m_deviceMemory;
        // VkImageType    m_imageType;
        // VkFormat       m_format;
        // uint32_t       m_width;
        // uint32_t       m_height;
        // uint32_t       m_depth;
        // uint32_t       m_mipLevels;
        // uint32_t       m_arrayLayers;

        std::shared_ptr<Image> m_image;
        TextureCreateInfo      m_info;
    };
} // namespace Chandelier