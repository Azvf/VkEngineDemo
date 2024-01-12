#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Chandelier {
	class Buffer
	{
	public:
		explicit Buffer(VkPhysicalDevice physicalDevice, VkDevice device, const VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags requiredFlags);
		~Buffer();

		Buffer(const Buffer&) = delete;
		Buffer(const Buffer&&) = delete;
		Buffer& operator= (const Buffer&) = delete;
		Buffer& operator= (const Buffer&&) = delete;
		
		VkBuffer getBuffer() const;
		VkDeviceMemory getMemory() const;
		size_t getSize() const;
		uint8_t* map();
		void unmap();

	private:
		void createBuffer(const VkBufferCreateInfo& bufferInfo, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VkDeviceSize m_size;
		uint8_t* m_mappedPtr;
		VkBuffer m_buffer;
		VkDeviceMemory m_memory;
	};


}