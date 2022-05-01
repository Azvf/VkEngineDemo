#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace vulkan
{
	class Image 
	{
	public:
		explicit Image(VkPhysicalDevice physicalDevice, VkDevice device, const VkImageCreateInfo& createInfo,
			VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, VkImageViewType viewType, const VkImageSubresourceRange& subresourceRange);
		~Image();
		
		Image(const Image&) = delete;
		Image(const Image&&) = delete;
		Image& operator= (const Image&) = delete;
		Image& operator= (const Image&&) = delete;
		
		const VkImage& getImage() const;
		const VkImageView& getView() const;
		const VkDeviceMemory& getMemory() const;

	private:
		void Image::createImage(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	
	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VkImage m_image;
		VkImageView m_view;
		VkDeviceMemory m_memory;
	};
}