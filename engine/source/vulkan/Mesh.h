#pragma once 

#include "VkCommon.h"

namespace Chandelier {
    namespace ShaderData
    {
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 tangent;
            glm::vec2 texcoord;

            static VkVertexInputBindingDescription getBindingDescription()
            {
                VkVertexInputBindingDescription bindingDescription {};
                bindingDescription.binding   = 0;
                bindingDescription.stride    = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return bindingDescription;
            }

            static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
            {
                std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions {};

                attributeDescriptions[0].binding  = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].offset   = offsetof(Vertex, position);

                attributeDescriptions[1].binding  = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].offset   = offsetof(Vertex, normal);
                 
                attributeDescriptions[2].binding  = 0;
                attributeDescriptions[2].location = 2;
                attributeDescriptions[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[2].offset   = offsetof(Vertex, tangent);

                attributeDescriptions[3].binding  = 0;
                attributeDescriptions[3].location = 3;
                attributeDescriptions[3].format   = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[3].offset   = offsetof(Vertex, texcoord);

                return attributeDescriptions;
            }

            bool operator==(const Vertex& other) const
            {
                return position == other.position && normal == other.normal && tangent == other.tangent && texcoord == other.texcoord;
            }
        };
    }

    enum BufferType : uint8_t
    {
        Vertex_Buffer = 0,
        Index_Buffer,
        Mesh_Buffer_Count,
    };

    enum MeshProperty : uint8_t
    {
        Has_Normal      = 1, 
        Has_UV          = 1 << 1, 
        Has_Tangent     = 1 << 2,
    };

	class Mesh {
	public:
        static std::shared_ptr<Mesh> load(std::shared_ptr<VKContext> context,
                                          uint32_t                   properties,
                                          const float*             vertices,
                                          size_t                     vertex_count,
                                          const uint32_t*            indices,
                                          size_t                     index_count);

        Mesh() = default;
		~Mesh();
        
		uint32_t getVertexCount() const;
		uint32_t getIndexCount() const;

        VkBuffer GetBuffer(BufferType type);

        std::vector<VkVertexInputBindingDescription> GetBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription {};
            bindingDescription.binding   = 0;
            bindingDescription.stride    = Mesh::GetVertexSize(m_properties);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return {bindingDescription};
        }

        std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
        {
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions {};

            VkVertexInputAttributeDescription desc = {};

            uint32_t offset = {};
            // position
            desc.binding  = 0;
            desc.location = 0;
            desc.format   = VK_FORMAT_R32G32B32_SFLOAT;
            desc.offset   = offset;
            attributeDescriptions.push_back(desc);
            offset += sizeof(float) * 3;

            if (m_properties & Has_Normal)
            {
                // noraml
                desc.binding  = 0;
                desc.location = 1;
                desc.format   = VK_FORMAT_R32G32B32_SFLOAT;
                desc.offset   = offset;
                attributeDescriptions.push_back(desc);
                offset += sizeof(float) * 3;
            }

            if (m_properties & Has_Tangent)
            {
                // tangent
                desc.binding  = 0;
                desc.location = 2;
                desc.format   = VK_FORMAT_R32G32B32_SFLOAT;
                desc.offset   = offset;
                attributeDescriptions.push_back(desc);
                offset += sizeof(float) * 3;
            }

            if (m_properties & Has_UV)
            {
                // uv
                desc.binding  = 0;
                desc.location = 3;
                desc.format   = VK_FORMAT_R32G32_SFLOAT;
                desc.offset   = offset;
                attributeDescriptions.push_back(desc);
                offset += sizeof(float) * 2;
            }

            return attributeDescriptions;
        }

        static size_t GetVertexSize(uint32_t properties);

	private:
        std::shared_ptr<VKContext> m_context;

		uint32_t m_vertexCount;
		uint32_t m_indexCount;
		
        std::vector<VkVertexInputBindingDescription>   m_bindings;
        std::vector<VkVertexInputAttributeDescription> m_attributes;

        std::array<std::unique_ptr<Buffer>, Mesh_Buffer_Count> m_buffers;

        uint32_t m_properties = {};
    };
}