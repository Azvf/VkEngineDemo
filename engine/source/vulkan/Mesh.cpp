#include "Mesh.h"

#include <vector>

#include "Buffer.h"
#include "CommandBuffers.h"
#include "VkContext.h"
#include "VkUtil.h"

namespace Chandelier
{
    VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool        = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkDevice        device,
                               VkQueue         graphicsQueue,
                               VkCommandPool   commandPool,
                               VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo {};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        assert(0);
        return 0;
    }

    void createBuffer(VkPhysicalDevice      physicalDevice,
                      VkDevice              device,
                      VkBufferCreateInfo    bufferInfo,
                      VkMemoryPropertyFlags properties,
                      VkBuffer&             buffer,
                      VkDeviceMemory&       bufferMemory)
    {
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            assert(0);
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            assert(0);
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

        void copyBuffer(VkDevice      device,
                    VkCommandPool commandPool,
                    VkQueue       graphicsQueue,
                    VkBuffer      srcBuffer,
                    VkBuffer      dstBuffer,
                    VkDeviceSize  size)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

        VkBufferCopy copyRegion {};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(device, graphicsQueue, commandPool, commandBuffer);
    }

    std::shared_ptr<Mesh> Mesh::load(std::shared_ptr<VKContext>   context,
                                     const std::vector<ShaderData::Vertex>& vertices,
                                     const std::vector<uint32_t>& indices)
    {
        //VkDeviceSize vertex_buffer_size = sizeof(vertices[0]) * vertices.size();
        //VkDeviceSize index_buffer_size  = sizeof(indices[0]) * indices.size();

        //auto mesh           = std::make_shared<Mesh>();
        //mesh->m_context     = context;
        //mesh->m_indexCount  = indices.size();
        //mesh->m_vertexCount = vertices.size();

        //const auto& phy_device = context->getPhysicalDevice();
        //const auto& device     = context->getDevice();

        //for (auto& buffer : mesh->m_buffers)
        //{
        //    buffer = std::make_unique<Buffer>();
        //}

        //auto& vertex_buffer = mesh->m_buffers[Vertex_Buffer];
        //auto& index_buffer  = mesh->m_buffers[Index_Buffer];

        //auto staging_buffer = std::make_unique<Buffer>();

        //staging_buffer->Allocate(context,
        //                         vertex_buffer_size + index_buffer_size,
        //                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        //                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        //                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        //                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        //staging_buffer->map();
        //staging_buffer->Update(
        //    reinterpret_cast<const uint8_t*>(vertices.data()), vertex_buffer_size);
        //staging_buffer->Update(
        //    reinterpret_cast<const uint8_t*>(indices.data()), index_buffer_size, 0, vertex_buffer_size);
        //staging_buffer->Flush();
        //staging_buffer->unmap();

        //vertex_buffer->Allocate(context,
        //                        vertex_buffer_size,
        //                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        //                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        //                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        //index_buffer->Allocate(context,
        //                       index_buffer_size,
        //                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        //                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        //                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        //std::vector<VkBufferCopy> buffers_copy = {{0, 0, vertex_buffer_size},
        //                                          {vertex_buffer_size, 0, index_buffer_size}};

        //auto& command_buffers = context->GetCommandBuffers();
        //command_buffers.Copy(
        //    staging_buffer.get(), vertex_buffer.get(), std::vector<VkBufferCopy> {buffers_copy[Vertex_Buffer]});
        //command_buffers.Copy(
        //    staging_buffer.get(), index_buffer.get(), std::vector<VkBufferCopy> {buffers_copy[Index_Buffer]});
        //command_buffers.Submit();

        VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
        VkDeviceSize indexBufferSize  = sizeof(indices[0]) * indices.size();

        auto mesh           = std::make_shared<Mesh>();
        mesh->m_indexCount         = indices.size();
        mesh->m_vertexCount        = vertices.size();
        const auto& device        = context->getDevice();
        const auto& physicalDevice = context->getPhysicalDevice();

        // vertex buffer
        {
            VkBufferCreateInfo createInfo {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            createInfo.size        = vertexBufferSize;
            createInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VkBuffer       stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(context->getPhysicalDevice(),
                         context->getDevice(),
                         createInfo,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, vertexBufferSize, 0, &data);
            memcpy(data, vertices.data(), (size_t)vertexBufferSize);
            vkUnmapMemory(device, stagingBufferMemory);

            createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            createBuffer(physicalDevice,
                         device,
                         createInfo,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         mesh->m_vertexBuffer,
                         mesh->m_vertexBufferMemory);

            copyBuffer(device,
                       context->GetCommandBuffers().GetCP(),
                       context->getGraphicsQueue(),
                       stagingBuffer,
                       mesh->m_vertexBuffer,
                       vertexBufferSize);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }

        // index buffer
        {
            VkBuffer       stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            VkBufferCreateInfo createInfo {};
            createInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            createInfo.size        = indexBufferSize;
            createInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            createBuffer(physicalDevice,
                         device,
                         createInfo,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, indexBufferSize, 0, &data);
            memcpy(data, indices.data(), (size_t)indexBufferSize);
            vkUnmapMemory(device, stagingBufferMemory);

            createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            createBuffer(physicalDevice,
                         device,
                         createInfo,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         mesh->m_indexBuffer,
                         mesh->m_indexBufferMemory);

            copyBuffer(device,
                       context->GetCommandBuffers().GetCP(),
                       context->getGraphicsQueue(),
                       stagingBuffer,
                       mesh->m_indexBuffer,
                       indexBufferSize);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }

        return mesh;
    }

    VkBuffer Mesh::GetBuffer(BufferType type) { /*return m_buffers[type]->getBuffer();*/
        if (type == BufferType::Vertex_Buffer)
        {
            return m_vertexBuffer;
        }
        else
        {
            return m_indexBuffer;
        }

        return m_vertexBuffer;
    }

    Mesh::~Mesh() {}

    uint32_t Mesh::getVertexCount() const { return m_vertexCount; }

    uint32_t Mesh::getIndexCount() const { return m_indexCount; }

} // namespace Chandelier
