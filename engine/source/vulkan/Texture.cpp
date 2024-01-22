#include "Texture.h"

#include <iostream>

#include "runtime/core/base/exception.h"

#include "Buffer.h"
#include "Image.h"
#include "VkContext.h"
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

        this->m_info = info;
    }

    std::shared_ptr<Texture> Texture::Create(uint8_t* image_data, const TextureCreateInfo& info)
    {
        auto tex = std::make_shared<Texture>();
        tex->Initialize(image_data, info);
        return tex;
    }

    VkImageView Texture::getView() const { return m_image->getView(); }

    VkImage Texture::getImage() const { return m_image->getImage(); }

    VkImageType Texture::getType() const { return VK_IMAGE_TYPE_2D; }

    VkFormat Texture::getFormat() const { return m_info.format; }

    uint32_t Texture::getWidth() const { return m_info.width; }

    uint32_t Texture::getHeight() const { return m_info.height; }

    uint32_t Texture::getDepth() const { return 1; }

    uint32_t Texture::getLevels() const { return 1; }

    uint32_t Texture::getLayers() const { return 1; }

} // namespace Chandelier