#pragma once

#include "VkCommon.h"

namespace Chandelier
{
    class Buffer
    {
    public:
        Buffer() = default;
        ~Buffer();

        Buffer(const Buffer&)             = delete;
        Buffer(const Buffer&&)            = delete;
        Buffer& operator=(const Buffer&)  = delete;
        Buffer& operator=(const Buffer&&) = delete;

        VkBuffer       getBuffer() const;
        VkDeviceMemory getMemory() const;
        size_t         getSize() const;
        uint8_t*       map();
        void           unmap();

        void Allocate(std::shared_ptr<VKContext> context,
                      VkDeviceSize               mem_size,
                      VkMemoryPropertyFlags      mem_props,
                      VkBufferUsageFlags         buffer_usage);

        bool Allocated();

    private:
        VkDeviceSize   m_size      = 0;
        uint8_t*       m_mappedPtr = nullptr;
        VkBuffer       m_buffer    = VK_NULL_HANDLE;
        VkDeviceMemory m_memory    = VK_NULL_HANDLE;

        VkMemoryPropertyFlags m_mem_props;
        VkBufferUsageFlags    m_buffer_usage;

        std::shared_ptr<VKContext> m_context;
    };

    class UniformBuffer
    {
        Buffer m_buffer;




    };


} // namespace Chandelier