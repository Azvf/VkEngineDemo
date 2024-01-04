#include "Buffer.h"

#include <iostream>

#include "VkUtil.h"

namespace Chandelier {
	Buffer::Buffer(VkPhysicalDevice physicalDevice, VkDevice device, const VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags requiredFlags)
		: m_physicalDevice(physicalDevice), m_device(device), m_size(createInfo.size), m_mappedPtr() 
	{
        createBuffer(createInfo, requiredFlags, m_buffer, m_memory);
	}

    Buffer::~Buffer() {
        vkDestroyBuffer(m_device, m_buffer, nullptr);
        vkFreeMemory(m_device, m_memory, nullptr);
    }

    void Buffer::createBuffer(const VkBufferCreateInfo& bufferInfo, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        if (vkBindBufferMemory(m_device, buffer, bufferMemory, 0) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }
    }

    VkBuffer Buffer::getBuffer() const
    {
        return m_buffer;
    }
    
    VkDeviceMemory Buffer::getMemory() const
    {
        return m_memory;
    }
    
    size_t Buffer::getSize() const
    {
        return m_size;
    }
    
    uint8_t* Buffer::map()
    {
        if (!m_mappedPtr)
        {
            if (vkMapMemory(m_device, m_memory, 0, m_size, 0, (void**)&m_mappedPtr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to map constant buffer memory!");
            }
        }
        return m_mappedPtr;
    }
    
    void Buffer::unmap()
    {
        if (m_mappedPtr)
        {
            vkUnmapMemory(m_device, m_memory);
            m_mappedPtr = nullptr;
        }
    }
}

