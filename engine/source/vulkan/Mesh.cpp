#include "Mesh.h"

#include <vector>

#include "Buffer.h"
#include "CommandBuffers.h"
#include "VkContext.h"

namespace Chandelier
{
    std::shared_ptr<Mesh> Mesh::load(std::shared_ptr<VKContext> context,
                                     uint32_t                   properties,
                                     const float*               vertices,
                                     size_t                     vertex_count,
                                     const uint32_t*            indices,
                                     size_t                     index_count)
    {
        auto mesh = std::make_shared<Mesh>();

        VkDeviceSize vertex_buffer_size = vertex_count * sizeof(float);
        VkDeviceSize index_buffer_size  = sizeof(uint32_t) * index_count;

        mesh->m_context     = context;
        mesh->m_indexCount  = index_count;
        mesh->m_vertexCount = vertex_buffer_size / Mesh::GetVertexSize(properties);
        mesh->m_properties  = properties;

        const auto& phy_device = context->getPhysicalDevice();
        const auto& device     = context->getDevice();

        for (auto& buffer : mesh->m_buffers)
        {
            buffer = std::make_unique<Buffer>();
        }

        auto& vertex_buffer = mesh->m_buffers[Vertex_Buffer];
        auto& index_buffer  = mesh->m_buffers[Index_Buffer];

        auto staging_buffer = std::make_unique<Buffer>();

        staging_buffer->Allocate(context,
                                 vertex_buffer_size + index_buffer_size,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        staging_buffer->map();
        staging_buffer->Update(reinterpret_cast<const uint8_t*>(vertices), vertex_buffer_size);
        if (indices)
        {
            staging_buffer->Update(reinterpret_cast<const uint8_t*>(indices), index_buffer_size, 0, vertex_buffer_size);
        }
        staging_buffer->Flush();
        staging_buffer->unmap();

        vertex_buffer->Allocate(context,
                                vertex_buffer_size,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        if (indices)
        {
            index_buffer->Allocate(context,
                                   index_buffer_size,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        }
        
        std::vector<VkBufferCopy> buffers_copy = {{0, 0, vertex_buffer_size},
                                                  {vertex_buffer_size, 0, index_buffer_size}};

        auto& command_manager = context->GetCommandManager();
        command_manager.Copy(
            staging_buffer.get(), vertex_buffer.get(), std::vector<VkBufferCopy> {buffers_copy[Vertex_Buffer]});
        
        if (indices)
        {
            command_manager.Copy(
                staging_buffer.get(), index_buffer.get(), std::vector<VkBufferCopy> {buffers_copy[Index_Buffer]});
        }

        command_manager.Submit();

        return mesh;
    }

    VkBuffer Mesh::GetBuffer(BufferType type) { return m_buffers[type]->getBuffer(); }

    Mesh::~Mesh() {}

    uint32_t Mesh::getVertexCount() const { return m_vertexCount; }

    uint32_t Mesh::getIndexCount() const { return m_indexCount; }

    size_t Mesh::GetVertexSize(uint32_t properties)
    {
        // position
        size_t vertex_size = sizeof(float) * 3;
        if (properties & Has_Normal)
        {
            vertex_size += sizeof(float) * 3;
        }
        
        if (properties & Has_UV)
        {
            vertex_size += sizeof(float) * 2;
        }
        
        if (properties & Has_Tangent)
        {
            vertex_size += sizeof(float) * 3;
        }

        return vertex_size;
    }

} // namespace Chandelier
