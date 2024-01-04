#pragma once 

#include <vector>
#include <memory>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Chandelier {
	class Vertex;

	class Mesh
	{
	public:
        static std::shared_ptr<Mesh> Mesh::load(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool cmdPool,
            const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        Mesh() = default;
		Mesh(const Mesh&) = delete;
		Mesh(const Mesh&&) = delete;
		Mesh& operator= (const Mesh&) = delete;
		Mesh& operator= (const Mesh&&) = delete;
		~Mesh();
		uint32_t getVertexCount() const;
		uint32_t getIndexCount() const;
		const VkBuffer& getVertexBuffer() const;
		const VkBuffer& getIndexBuffer() const;

	private:
		VkDevice m_device;
		uint32_t m_vertexCount;
		uint32_t m_indexCount;
		VkBuffer m_vertexBuffer;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		VkDeviceMemory m_indexBufferMemory;
	};
}