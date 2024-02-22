#pragma once

#include "ShaderResource.h"
#include "VkCommon.h"

namespace Chandelier
{
    class Buffer
    {
    public:
        Buffer() = default;
        ~Buffer();

        VkBuffer       getBuffer() const;
        VkDeviceMemory getMemory() const;
        VkDeviceSize   getSize() const;
        VkDeviceSize   getOffset() const;
        uint8_t*       map();
        void           unmap();

        bool IsMapped();
        void Update(const uint8_t* data, size_t size, uint64_t src_offset = 0, uint64_t dst_offset = 0);
        void Flush();

        bool IsAllocated();
        void Allocate(std::shared_ptr<VKContext> context,
                      VkDeviceSize               mem_size,
                      VkMemoryPropertyFlags      mem_props,
                      VkBufferUsageFlags         buffer_usage,
                      VkDescriptorType           bind_type);

        VkDescriptorType GetBindType();
        VkBufferUsageFlags GetUsage();

    protected:
        VkDeviceSize     m_size      = {};
        VkDeviceSize     m_offset    = {};
        uint8_t*         m_mappedPtr = nullptr;
        VkBuffer         m_buffer    = VK_NULL_HANDLE;
        VkDeviceMemory   m_memory    = VK_NULL_HANDLE;
        /**
         * @todo: change to optional
         */
        VkDescriptorType m_bind_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;

        VkMemoryPropertyFlags m_mem_props;
        VkBufferUsageFlags    m_buffer_usage;

        std::shared_ptr<VKContext> m_context;
    };
} // namespace Chandelier