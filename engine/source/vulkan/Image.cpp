#include "Image.h"

#include <iostream>

#include "runtime/core/base/exception.h"

#include "VkContext.h"
#include "VkUtil.h"

namespace Chandelier
{
    Image::~Image() {}

    VkImageCreateInfo Image::ToVkImageCreateInfo(const ImageCreateInfo& info)
    {
        VkImageCreateInfo image_create_info {};
        image_create_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.flags         = info.create_flags;
        image_create_info.imageType     = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width  = info.width;
        image_create_info.extent.height = info.height;
        image_create_info.extent.depth  = 1;
        image_create_info.mipLevels     = info.mip_levels;
        image_create_info.arrayLayers   = info.array_layers;
        image_create_info.format        = info.format;
        image_create_info.tiling        = info.tiling;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.usage         = info.usage_flags;
        image_create_info.samples       = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

        return image_create_info;
    }

    void Image::Initialize(const ImageCreateInfo& info)
    {
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

        m_info        = info;
        m_info.layout = layout;

        const auto& device = info.context->getDevice();

        VkImageCreateInfo image_create_info = ToVkImageCreateInfo(info);
        VULKAN_API_CALL(vkCreateImage(device, &image_create_info, nullptr, &m_image));

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, m_image, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =
            findMemoryType(info.context->getPhysicalDevice(), memRequirements.memoryTypeBits, info.mem_flags);

        VULKAN_API_CALL(vkAllocateMemory(device, &allocInfo, nullptr, &m_memory));
        VULKAN_API_CALL(vkBindImageMemory(device, m_image, m_memory, 0));

        VkImageViewCreateInfo view_info {};
        view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image                           = m_image;
        view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format                          = info.format;
        view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel   = 0;
        view_info.subresourceRange.levelCount     = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount     = 1;

        VULKAN_API_CALL(vkCreateImageView(device, &view_info, nullptr, &m_view));
    }

    std::shared_ptr<Image> Image::Create(const ImageCreateInfo& info)
    {
        auto image = std::make_shared<Image>();
        image->Initialize(info);
        return image;
    }

    Image::~Image()
    {
        if (m_info.context)
        {
            const auto& device = m_info.context->getDevice();
            vkDestroyImageView(device, m_view, nullptr);
            vkDestroyImage(device, m_image, nullptr);
        }
    }

    const VkImage& Image::getImage() const { return m_image; }

    const VkImageView& Image::getView() const { return m_view; }

    const VkDeviceMemory& Image::getMemory() const { return m_memory; }

} // namespace Chandelier
