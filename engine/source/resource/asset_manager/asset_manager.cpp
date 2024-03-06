#include "asset_manager.h"

#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "render/base/common_vao_defines.h"
#include "runtime/framework/config_manager/config_manager.h"

#include "VkUtil.h"
#include "VkContext.h"
#include "Mesh.h"
#include "Texture.h"
#include "Buffer.h"
#include "CommandBuffers.h"

namespace std
{
    template<>
    struct hash<Chandelier::ShaderData::Vertex>
    {
        size_t operator()(Chandelier::ShaderData::Vertex const& vertex) const
        {
            // Include the tangent parameter in the hash calculation
            return (((((hash<glm::vec3>()(vertex.position) ^
                        (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                        (hash<glm::vec2>()(vertex.texcoord) << 1)) ^
                        (hash<glm::vec3>()(vertex.tangent) << 2))); // Assuming tangent is a glm::vec3
        }
    };
} // namespace std

namespace Chandelier
{
    std::filesystem::path GetFullPath(std::string_view relative_path) 
    {
        return std::filesystem::absolute(relative_path);
    }

    std::shared_ptr<Mesh> LoadDefaultMesh(std::shared_ptr<VKContext> context, DefaultMeshType mesh_type)
    {
        if (mesh_type == Screen_Mesh)
        {
            return Mesh::load(context, Has_UV, ScreenVertices.data(), ScreenVertices.size(), nullptr, 0);
        }
        else if (mesh_type == Cube_Mesh)
        {
            return Mesh::load(context, Has_Normal | Has_UV, CubeVertices.data(), CubeVertices.size(), nullptr, 0);
        }
        return nullptr;
    }

    std::shared_ptr<Mesh> LoadStaticMesh(std::shared_ptr<VKContext> context, std::string_view filename)
    {
        auto mesh = std::make_shared<Mesh>();

        tinyobj::ObjReader       reader;
        tinyobj::ObjReaderConfig reader_config;
        reader_config.vertex_color = false;
        if (!reader.ParseFromFile(filename.data(), reader_config))
        {
            if (!reader.Error().empty())
            {
                auto error = reader.Error();
                assert(0);
            }
        }

        if (!reader.Warning().empty())
        {
            auto warning = reader.Warning();
            // assert(0);
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();

        std::unordered_map<ShaderData::Vertex, uint32_t> unique_vertices;

        std::vector<ShaderData::Vertex> mesh_vertices;
        std::vector<uint32_t>           mesh_indices;

        uint32_t mesh_properties = {};
        if (attrib.normals.size())
        {
            mesh_properties |= (Has_Normal | Has_Tangent);
        }
        if (attrib.texcoords.size())
        {
            mesh_properties |= Has_UV;
        }

        for (size_t s = 0; s < shapes.size(); s++)
        {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
            {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                bool with_normal   = true;
                bool with_texcoord = true;

                Vector3 vertex[3];
                Vector3 normal[3];
                Vector2 uv[3];

                // only deals with triangle faces
                if (fv != 3)
                {
                    assert(0);
                    continue;
                }

                // expanding vertex data is not efficient and is for testing purposes only
                for (size_t v = 0; v < fv; v++)
                {
                    auto idx = shapes[s].mesh.indices[index_offset + v];
                    auto vx  = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    auto vy  = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    auto vz  = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                    vertex[v].x = static_cast<float>(vx);
                    vertex[v].y = static_cast<float>(vy);
                    vertex[v].z = static_cast<float>(vz);

                    if (idx.normal_index >= 0)
                    {
                        auto nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        auto ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        auto nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                        normal[v].x = static_cast<float>(nx);
                        normal[v].y = static_cast<float>(ny);
                        normal[v].z = static_cast<float>(nz);
                    }
                    else
                    {
                        with_normal = false;
                    }

                    if (idx.texcoord_index >= 0)
                    {
                        auto tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        auto ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                        uv[v].x = static_cast<float>(tx);
                        uv[v].y = static_cast<float>(ty);
                    }
                    else
                    {
                        with_texcoord = false;
                    }
                }
                index_offset += fv;

                if (!with_normal)
                {
                    Vector3 v0 = vertex[1] - vertex[0];
                    Vector3 v1 = vertex[2] - vertex[1];
                    normal[0]  = v0.crossProduct(v1).normalisedCopy();
                    normal[1]  = normal[0];
                    normal[2]  = normal[0];
                }

                if (!with_texcoord)
                {
                    uv[0] = Vector2(0.5f, 0.5f);
                    uv[1] = Vector2(0.5f, 0.5f);
                    uv[2] = Vector2(0.5f, 0.5f);
                }

                Vector3 tangent {1, 0, 0};
                {
                    Vector3 edge1    = vertex[1] - vertex[0];
                    Vector3 edge2    = vertex[2] - vertex[1];
                    Vector2 deltaUV1 = uv[1] - uv[0];
                    Vector2 deltaUV2 = uv[2] - uv[1];

                    auto divide = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
                    if (divide >= 0.0f && divide < 0.000001f)
                        divide = 0.000001f;
                    else if (divide < 0.0f && divide > -0.000001f)
                        divide = -0.000001f;

                    float df  = 1.0f / divide;
                    tangent.x = df * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    tangent.y = df * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    tangent.z = df * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    tangent   = tangent.normalisedCopy();
                }

                for (size_t i = 0; i < 3; i++)
                {
                    ShaderData::Vertex mesh_vert {};

                    mesh_vert.position.x = vertex[i].x;
                    mesh_vert.position.y = vertex[i].y;
                    mesh_vert.position.z = vertex[i].z;

                    mesh_vert.normal.x = normal[i].x;
                    mesh_vert.normal.y = normal[i].y;
                    mesh_vert.normal.z = normal[i].z;

                    mesh_vert.texcoord.x = uv[i].x;
                    mesh_vert.texcoord.y = uv[i].y;

                    mesh_vert.tangent.x = tangent.x;
                    mesh_vert.tangent.y = tangent.y;
                    mesh_vert.tangent.z = tangent.z;

                    if (!unique_vertices.count(mesh_vert))
                    {
                        unique_vertices[mesh_vert] = static_cast<uint32_t>(mesh_vertices.size());
                        mesh_vertices.push_back(mesh_vert);
                    }

                    mesh_indices.push_back(unique_vertices[mesh_vert]);
                }
            }
        }

        std::vector<float> input_vertices;
        for (auto& v : mesh_vertices)
        {
            input_vertices.push_back(v.position.x);
            input_vertices.push_back(v.position.y);
            input_vertices.push_back(v.position.z);

            input_vertices.push_back(v.normal.x);
            input_vertices.push_back(v.normal.y);
            input_vertices.push_back(v.normal.z);

            input_vertices.push_back(v.tangent.x);
            input_vertices.push_back(v.tangent.y);
            input_vertices.push_back(v.tangent.z);

            input_vertices.push_back(v.texcoord.x);
            input_vertices.push_back(v.texcoord.y);
        }
        return mesh->load(context,
                          mesh_properties,
                          input_vertices.data(),
                          input_vertices.size(),
                          mesh_indices.data(),
                          mesh_indices.size());
    }

    static VkFormat CubeMapChannelsToFormat(int desired_channels) {
        VkFormat format;
        switch (desired_channels)
        {
            case 2:
                format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case 4:
                format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            default:
                /**
                 * @warning: three component format is not supported in some vulkan driver implementations
                 */
                assert(0);
                throw std::runtime_error("unsupported channels number");
                break;
        }
        return format;
    }

    std::shared_ptr<Texture>
    LoadTexture(std::shared_ptr<VKContext> context, std::string_view path, ColorSpace color_space)
    {
        int texWidth, texHeight, texChannels;
        
        stbi_uc*     pixels    = stbi_load(path.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels)
        {
            assert(0);
            return std::shared_ptr<Texture>();
        }

        VkFormat format;
        switch (color_space)
        {
            case SRGB_Color_Space:
                format = VK_FORMAT_R8G8B8A8_SRGB;
                break;
            case Linear_Color_Space:
                format = VK_FORMAT_R8G8B8A8_UNORM;
                break;
            default:
                assert(0);
                break;
        }

        auto texture = std::make_shared<Texture>();
        texture->InitTex2D(context,
                           texWidth,
                           texHeight,
                           1,
                           1,
                           format,
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

    std::shared_ptr<Texture> LoadSkybox(std::shared_ptr<VKContext> context, std::string_view path, int desired_channels)
    {
        assert(0 && "wip");
        auto texture = std::make_shared<Texture>();
        return texture;
    }

    std::shared_ptr<Texture>
    LoadSkybox(std::shared_ptr<VKContext> context, std::array<std::shared_ptr<Texture>, 6> faces, int desired_channels)
    {
        auto     texture = std::make_shared<Texture>();
        VkFormat format  = CubeMapChannelsToFormat(desired_channels);

        int width = faces.front()->getWidth();
        int height = faces.front()->getHeight();
        
        uint32_t cubemap_miplevels = std::floor(log2(std::max(width, height))) + 1;
        texture->InitCubeMap(context, faces, cubemap_miplevels, format);
        
        texture->TransferLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                0,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_ACCESS_TRANSFER_WRITE_BIT);

        std::vector<std::shared_ptr<Texture>> face_vec(faces.begin(), faces.end());
        
        for (auto& face : faces)
        {
            face->TransferLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 0,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_ACCESS_TRANSFER_WRITE_BIT);
        }
        
        texture->Sync(face_vec);

        /**
         * @todo: in generate processes target texture already transfered to perferred layout, 
         * right now we don't care about faces cause they're gonna get destroyed
         */
        context->GenerateMipMaps(texture.get(), cubemap_miplevels);

        return texture;
    }

    std::shared_ptr<Texture> LoadTextureHDR(std::shared_ptr<VKContext> context, std::string_view path, int desired_channels)
    {
        auto texture = std::make_shared<Texture>();
        
        int tex_width, tex_height, tex_channels;
        float* pixels = stbi_loadf(path.data(), &tex_width, &tex_height, &tex_channels, desired_channels);

        if (!pixels)
        {
            assert(0);
            return texture;
        }

        VkFormat format = CubeMapChannelsToFormat(desired_channels);

        texture->InitTex2D(context,
                           tex_width,
                           tex_height,
                           1,
                           1,
                           format,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
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

    void SaveTexture(std::shared_ptr<Texture> texture, std::string_view path, uint32_t layer, uint32_t mip_level)
    {
        const uint8_t* pixels   = texture->Data(layer, mip_level);
        uint32_t       width    = texture->getWidth() * std::pow(0.5, mip_level);
        uint32_t       height   = texture->getHeight() * std::pow(0.5, mip_level);
        uint32_t       channels = 4;
        int            result   = stbi_write_png(path.data(), width, height, channels, pixels, width * channels);
        assert(result);
    }

    void SaveHDRTexture(std::shared_ptr<Texture> texture, std::string_view path, uint32_t layer, uint32_t mip_level)
    {
        const float*   pixels   = (float*)texture->Data(layer, mip_level);
        uint32_t       width    = texture->getWidth() * std::pow(0.5, mip_level);
        uint32_t       height   = texture->getHeight() * std::pow(0.5, mip_level);
        uint32_t       channels = 4;
        int            result   = stbi_write_hdr(path.data(), width, height, channels, pixels);
        assert(result);
    }

}