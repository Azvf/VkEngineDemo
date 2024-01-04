#include "Image.h"

#include <iostream>

#include "VkUtil.h"

namespace Chandelier {
	
	Image::Image(VkPhysicalDevice physicalDevice, VkDevice device, const VkImageCreateInfo& createInfo, 
		VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, VkImageViewType viewType, const VkImageSubresourceRange& subresourceRange)
		: m_physicalDevice(physicalDevice), m_device(device)
	{
		createImage(m_physicalDevice, m_device, createInfo, requiredFlags, m_image, m_memory);
	    
        VkImageViewCreateInfo viewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewCreateInfo.viewType = viewType;
        viewCreateInfo.image = m_image;
        viewCreateInfo.format = createInfo.format;
        viewCreateInfo.subresourceRange = subresourceRange;

        if (vkCreateImageView(m_device, &viewCreateInfo, nullptr, &m_view) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image view!");
        }
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

