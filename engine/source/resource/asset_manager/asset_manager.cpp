#include "asset_manager.h"

#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "runtime/framework/config_manager/config_manager.h"

#include "VkUtil.h"
#include "VkContext.h"
#include "Mesh.h"
#include "Texture.h"
#include "CommandBuffers.h"

namespace std
{
    template<>
    struct hash<Chandelier::ShaderData::Vertex>
    {
        size_t operator()(Chandelier::ShaderData::Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
} // namespace std

namespace Chandelier
{
    std::filesystem::path GetFullPath(std::string_view relative_path) 
    {
        return std::filesystem::absolute(relative_path);
    }

    std::shared_ptr<Mesh> LoadObjModel(std::shared_ptr<VKContext> context, std::string_view path)
    {
        tinyobj::attrib_t                attrib;
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;
        std::string                      warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.data()))
        {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<ShaderData::Vertex, uint32_t> uniqueVertices {};

        std::vector<ShaderData::Vertex> vertices;
        std::vector<uint32_t> indices;

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                ShaderData::Vertex vertex {};

                vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                              attrib.vertices[3 * index.vertex_index + 1],
                              attrib.vertices[3 * index.vertex_index + 2]};

                vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
                                   1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        return Mesh::load(context, vertices, indices);
    }

    std::shared_ptr<Texture> LoadTexture(std::shared_ptr<VKContext> context, std::string_view path)
    {
        int texWidth, texHeight, texChannels;
        
        stbi_uc*     pixels    = stbi_load(path.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels)
        {
            assert(0);
            return std::shared_ptr<Texture>();
        }

        auto texture = std::make_shared<Texture>();
        texture->InitTex2D(context,
                           texWidth,
                           texHeight,
                           1,
                           1,
                           VK_FORMAT_R8G8B8A8_SRGB,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        texture->TransferLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                0,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_ACCESS_TRANSFER_WRITE_BIT);
        
        texture->Sync(reinterpret_cast<const uint8_t*>(pixels));
        
        texture->TransferLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_ACCESS_TRANSFER_WRITE_BIT,
                                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
                                0);
        
        context->GetCommandManager().Submit();

        return texture;
    }

}