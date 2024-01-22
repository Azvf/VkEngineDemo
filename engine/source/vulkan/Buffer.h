#pragma once

#include <memory>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "VkCreateInfo.h"

namespace Chandelier {
	class Buffer {
	public:
		Buffer() = default;
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

		void Initialize(const BufferCreateInfo& info);
		static std::shared_ptr<Buffer> Create(const BufferCreateInfo& info);

		friend class VKContext;

	private:
		void createBuffer(const VkBufferCreateInfo& bufferInfo, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	private:
		VkPhysicalDevice	m_physicalDevice;
		VkDevice			m_device;
		VkDeviceSize		m_size;
		uint8_t*			m_mappedPtr;
		VkBuffer			m_buffer;
		VkDeviceMemory		m_memory;

		BufferCreateInfo	m_info;
	};


}