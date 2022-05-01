#include "Image.h"

#include <iostream>

#include "VkUtil.h"

namespace vulkan {
	
	Image::Image(VkPhysicalDevice physicalDevice, VkDevice device, const VkImageCreateInfo& createInfo, 
		VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, VkImageViewType viewType, const VkImageSubresourceRange& subresourceRange)
		: m_physicalDevice(physicalDevice), m_device(device)
	{
		createImage(createInfo, requiredFlags, m_image, m_memory);
	}

    void Image::createImage(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        
        if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(m_device, image, imageMemory, 0);
    }

    Image::~Image() {
        // vkDestroySampler(device, textureSampler, nullptr);
        vkDestroyImageView(m_device, m_view, nullptr);
        vkDestroyImage(m_device, m_image, nullptr);
    }

    const VkImage& Image::getImage() const
    {
        return m_image;
    }

    const VkImageView& Image::getView() const
    {
        return m_view;
    }

    const VkDeviceMemory& Image::getMemory() const
    {
        return m_memory;
    }

}

