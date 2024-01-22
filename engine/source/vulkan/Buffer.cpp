#include "Buffer.h"

#include <iostream>

#include "runtime/core/base/exception.h"

#include "VkUtil.h"
#include "VkContext.h"

namespace Chandelier {
    Buffer::~Buffer() {
        if (m_info.context) {
            const auto& device = m_info.context->getDevice();
            vkDestroyBuffer(device, m_buffer, nullptr);
            vkFreeMemory(device, m_memory, nullptr);
        }
    }

    void Buffer::Initialize(const BufferCreateInfo& info) {
        const auto& device = info.context->getDevice();
        VULKAN_API_CALL(vkCreateBuffer(device, &info.vk_buffer_info, nullptr, &m_buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, m_buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, info.mem_flags);

        VULKAN_API_CALL(vkAllocateMemory(device, &allocInfo, nullptr, &m_memory));
        VULKAN_API_CALL(vkBindBufferMemory(device, m_buffer, m_memory, 0));

        this->m_info = info;
    }

    std::shared_ptr<Buffer> Buffer::Create(const BufferCreateInfo& info) {
        auto buffer = std::make_shared<Buffer>();
        buffer->Initialize(info);
        return buffer;
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

