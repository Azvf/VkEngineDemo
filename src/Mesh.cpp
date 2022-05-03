#include "Mesh.h"

#include "VkUtil.h"
#include <vector>

namespace vulkan {

	std::shared_ptr<Mesh> Mesh::load(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool cmdPool,
		const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
		VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();

		auto mesh = std::make_shared<Mesh>();
		mesh->m_device = device;
		mesh->m_indexCount = indexBufferSize;
		mesh->m_vertexCount = vertexBufferSize;

		// vertex buffer
		{
			VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			createInfo.size = vertexBufferSize;
			createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			createBuffer(physicalDevice, device, createInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, vertexBufferSize, 0, &data);
			memcpy(data, vertices.data(), (size_t)vertexBufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			createBuffer(physicalDevice, device, createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh->m_vertexBuffer, mesh->m_vertexBufferMemory);

			copyBuffer(device, cmdPool, queue, stagingBuffer, mesh->m_vertexBuffer, vertexBufferSize);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
		}

		// index buffer
		{
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;

			VkBufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			createInfo.size = indexBufferSize;
			createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			createBuffer(physicalDevice, device, createInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, indexBufferSize, 0, &data);
			memcpy(data, indices.data(), (size_t)indexBufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			createBuffer(physicalDevice, device, createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh->m_indexBuffer, mesh->m_indexBufferMemory);

			copyBuffer(device, cmdPool, queue, stagingBuffer, mesh->m_indexBuffer, indexBufferSize);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
		}

		return mesh;
	}

	Mesh::~Mesh() {
		vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
		vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
		vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
		vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
	}

	uint32_t Mesh::getVertexCount() const
	{
		return m_vertexCount;
	}

	uint32_t Mesh::getIndexCount() const
	{
		return m_indexCount;
	}

	VkBuffer Mesh::getVertexBuffer() const
	{
		return m_vertexBuffer;
	}

	VkBuffer Mesh::getIndexBuffer() const
	{
		return m_indexBuffer;
	}

}

