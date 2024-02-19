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
                          VkBufferUsageFlags         buffer_usage,
                          VkDescriptorType           bind_type)
    {
        m_context      = context;
        m_bind_type    = bind_type;
        m_mem_props    = mem_props;
        m_buffer_usage = buffer_usage;

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.flags = 0;
        /*
         * Vulkan doesn't allow empty buffers but some areas (DrawManager Instance data, PyGPU)
         * create them.
         */
        buffer_info.size  = std::max(mem_size, (VkDeviceSize)1);
        buffer_info.usage = buffer_usage;
        /* We use the same command queue for the compute and graphics pipeline, so it is safe to use
         * exclusive resource handling. */
        buffer_info.sharingMode                   = VK_SHARING_MODE_EXCLUSIVE;
        buffer_info.queueFamilyIndexCount         = 1;
        QueueFamilyIndices queue_families         = m_context->FindQueueFamilies(m_context->getPhysicalDevice());
        const uint32_t     queue_family_indices[] = {queue_families.graphicsFamily.value(),
                                                     queue_families.presentFamily.value()};
        buffer_info.pQueueFamilyIndices           = queue_family_indices;

        const auto& device = context->getDevice();
        VULKAN_API_CALL(vkCreateBuffer(device, &buffer_info, nullptr, &m_buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, m_buffer, &memRequirements);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize       = memRequirements.size;
        alloc_info.memoryTypeIndex      =
            m_context->FindMemoryType(memRequirements.memoryTypeBits, mem_props);

        VULKAN_API_CALL(vkAllocateMemory(device, &alloc_info, nullptr, &m_memory));
        VULKAN_API_CALL(vkBindBufferMemory(device, m_buffer, m_memory, 0));
        
        m_size = memRequirements.size;
    }

    VkBuffer Buffer::getBuffer() const { return m_buffer; }

    VkDeviceMemory Buffer::getMemory() const { return m_memory; }

    VkDeviceSize Buffer::getSize() const { return m_size; }

    VkDeviceSize Buffer::getOffset() const { return m_offset; }

    uint8_t* Buffer::map()
    {
        assert(!m_mappedPtr);
        VULKAN_API_CALL(vkMapMemory(m_context->getDevice(), m_memory, 0, m_size, 0, (void**)&m_mappedPtr));
        return m_mappedPtr;
    }

    void Buffer::unmap()
    {
        assert(m_mappedPtr);
        vkUnmapMemory(m_context->getDevice(), m_memory);
        m_mappedPtr = nullptr;
    }

    void Buffer::Update(const uint8_t* data, size_t size, uint64_t src_offset, uint64_t dst_offset)
    {
        assert(data);

        if (!IsMapped())
        {
            assert(0);
            return;
        }

        memcpy(m_mappedPtr + dst_offset, data + src_offset, size);
    }

    void Buffer::Flush() 
    { 
        m_context->FlushMappedBuffers(std::vector<Buffer*> {this}); 
    }

    bool Buffer::IsAllocated() { return m_memory != VK_NULL_HANDLE; }

    bool Buffer::IsMapped() { return m_mappedPtr; }
    
    VkDescriptorType Buffer::GetBindType() { return m_bind_type; }
    
    VkBufferUsageFlags Buffer::GetUsage() { return m_buffer_usage; }

} // namespace Chandelier
