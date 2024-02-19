#pragma once 

#include "VkCommon.h"

namespace Chandelier {
    namespace ShaderData
    {
        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 color;
            glm::vec2 texCoord;

            static VkVertexInputBindingDescription getBindingDescription()
            {
                VkVertexInputBindingDescription bindingDescription {};
                bindingDescription.binding   = 0;
                bindingDescription.stride    = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return bindingDescription;
            }

            static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
            {
                std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {};

                attributeDescriptions[0].binding  = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].offset   = offsetof(Vertex, pos);

                attributeDescriptions[1].binding  = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].offset   = offsetof(Vertex, color);

                attributeDescriptions[2].binding  = 0;
                attributeDescriptions[2].location = 2;
                attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[2].offset   = offsetof(Vertex, texCoord);

                return attributeDescriptions;
            }

            bool operator==(const Vertex& other) const
            {
                return pos == other.pos && color == other.color && texCoord == other.texCoord;
            }
        };
    }

    enum BufferType : uint8_t
    {
        Vertex_Buffer = 0,
        Index_Buffer,
        Mesh_Buffer_Count,
    };

	class Mesh {
	public:
        static std::shared_ptr<Mesh> load(std::shared_ptr<VKContext> context,
                                          const std::vector<ShaderData::Vertex>& vertices,
                                          const std::vector<uint32_t>&           indices);
        Mesh() = default;
		~Mesh();
        
		uint32_t getVertexCount() const;
		uint32_t getIndexCount() const;

        VkBuffer GetBuffer(BufferType type);

	private:
        std::shared_ptr<VKContext> m_context;

		uint32_t m_vertexCount;
		uint32_t m_indexCount;
		
        std::array<std::unique_ptr<Buffer>, Mesh_Buffer_Count> m_buffers;
	    
        
        VkBuffer       m_vertexBuffer;
        VkBuffer       m_indexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        VkDeviceMemory m_indexBufferMemory;

    };
}