#include "Buffer.h"

#include <iostream>

#include "runtime/core/base/exception.h"

#include "VkContext.h"
#include "VkUtil.h"

namespace Chandelier
{
    Buffer::~Buffer()
    {
        if (m_context)
        {
            const auto& device = m_context->getDevice();
            vkDestroyBuffer(device, m_buffer, nullptr);
            vkFreeMemory(device, m_memory, nullptr);
        }
    }

    void Buffer::Allocate(std::shared_ptr<VKContext> context,
                          VkDeviceSize               mem_size,
                          VkMemoryPropertyFlags      mem_props,
                          VkBufferUsageFlags         buffer_usage)
    {
        m_context = context;

        VkBufferCreateInfo buffer_info;
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.flags = 0;
        /*
         * Vulkan doesn't allow empty buffers but some areas (DrawManager Instance data, PyGPU)
         * create them.
         */
        buffer_info.size  = std::max((int)mem_size, 1);
        buffer_info.usage = buffer_usage;
        /* We use the same command queue for the compute and graphics pipeline, so it is safe to use
         * exclusive resource handling. */
        buffer_info.sharingMode                   = VK_SHARING_MODE_EXCLUSIVE;
        buffer_info.queueFamilyIndexCount         = 1;
        QueueFamilyIndices queue_families         = m_context->FindQueueFamilies();
        const uint32_t     queue_family_indices[] = {queue_families.graphicsFamily.value(),
                                                     queue_families.presentFamily.value()};
        buffer_info.pQueueFamilyIndices           = queue_family_indices;

        const auto& device = context->getDevice();
        VULKAN_API_CALL(vkCreateBuffer(device, &buffer_info, nullptr, &m_buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, m_buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =
            m_context->FindMemoryType(memRequirements.memoryTypeBits, mem_props);

        VULKAN_API_CALL(vkAllocateMemory(device, &allocInfo, nullptr, &m_memory));
        VULKAN_API_CALL(vkBindBufferMemory(device, m_buffer, m_memory, 0));
    }

    VkBuffer Buffer::getBuffer() const { return m_buffer; }

    VkDeviceMemory Buffer::getMemory() const { return m_memory; }

    size_t Buffer::getSize() const { return m_size; }

    uint8_t* Buffer::map()
    {
        if (!m_mappedPtr)
        {
            if (vkMapMemory(m_context->getDevice(), m_memory, 0, m_size, 0, (void**)&m_mappedPtr) !=
                VK_SUCCESS)
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
            vkUnmapMemory(m_context->getDevice(), m_memory);
            m_mappedPtr = nullptr;
        }
    }

    bool Buffer::Allocated() { return m_memory != VK_NULL_HANDLE; }


} // namespace Chandelier
