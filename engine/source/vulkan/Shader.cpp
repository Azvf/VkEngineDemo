#include "Shader.h"

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "runtime/core/base/exception.h"

#include "VkContext.h"

#include "common_utils.h"

namespace Chandelier
{
    Shader::~Shader() { UnInit(); }

    void Shader::Initialize(std::shared_ptr<VKContext> context,
                            VkShaderStageFlagBits      stage,
                            const uint8_t*             code,
                            uint64_t                   size)
    {
        m_context = context;
        m_shader_stage = stage;

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = size;
        createInfo.pCode    = reinterpret_cast<const uint32_t*>(code);
        
        VULKAN_API_CALL(vkCreateShaderModule(context->getDevice(), &createInfo, nullptr, &m_shader_module));
    }

    void Shader::UnInit()
    {
        if (m_shader_module != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(m_context->getDevice(), m_shader_module, nullptr);
        }
        m_shader_module = VK_NULL_HANDLE;
    }

    GraphicsPipelineShaders::~GraphicsPipelineShaders() { UnInit(); }

    void GraphicsPipelineShaders::Initialize(std::shared_ptr<VKContext> context) { m_context = context; }

    void GraphicsPipelineShaders::UnInit() {}

    void GraphicsPipelineShaders::InitShader(std::string_view shader_path, ShaderStage stage)
    {
        auto shader = std::make_optional<Shader>();
        auto code = readBinaryFile(shader_path.data());
        shader->Initialize(
            m_context, ShaderStageToVkStage(stage), reinterpret_cast<const uint8_t*>(code.data()), code.size());
        
        m_shaders[stage] = shader;

        spirv_cross::Compiler        compiler(reinterpret_cast<const uint32_t*>(code.data()), code.size() / sizeof(uint32_t));
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        auto read_resource = [&](auto resource, VkDescriptorType desc_type) {
            std::string& name = resource.name;

            if (m_bind_meta_map.find(resource.name) == m_bind_meta_map.end())
            {
                uint32_t                     set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                uint32_t                     binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                const spirv_cross::SPIRType& type    = compiler.get_type(resource.type_id);
                uint32_t                     typeArraySize = type.array.size();
                uint32_t                     count         = typeArraySize == 0 ? 1 : type.array[0];
                BindMetaData                 metaData {set, binding, count, desc_type, stage};
                m_bind_meta_map[name] = metaData;
            }
            else
            {
                m_bind_meta_map[name].shader_stage_flag |= stage;
            }
        };

        for (auto& resource : resources.uniform_buffers)
        {
            read_resource(resource, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }

        for (auto& resource : resources.sampled_images)
        {
            read_resource(resource, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        }

        for (auto& resource : resources.separate_samplers)
        {
            read_resource(resource, VK_DESCRIPTOR_TYPE_SAMPLER);
        }

        for (auto& resource : resources.separate_images)
        {
            read_resource(resource, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
        }

        for (auto& resource : resources.subpass_inputs)
        {
            read_resource(resource, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        }

        for (const auto& resource : resources.push_constant_buffers)
        {
            const std::string&           name = resource.name;
            const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
            uint32_t                     size = compiler.get_declared_struct_size(type);
            if (!m_push_const_data.has_value())
            {
                PushConstMetaData meta {size, 0, stage};
                m_push_const_data.emplace(meta);
            }
            else
            {
                m_push_const_data->shader_stage_flag |= stage;
            }
        }

        // std::shared_ptr<DescriptorTracker> m_desc_tracker;
        // auto& default_sampler = m_context->GetSampler(GPUSamplerState::default_sampler());
        // 
        // size_t index = 0;
        // m_desc_tracker->Bind(m_ubo.get(), Location(index++), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        // 
        // for (auto& texture : m_pass_info->render_resources->model_tex_vec)
        // {
        //     m_desc_tracker->Bind(texture.get(), &default_sampler, Location(index++), VK_SHADER_STAGE_FRAGMENT_BIT);
        // }
        // 
        // auto& cubemap_sampler = m_context->GetSampler(GPUSamplerState::cubemap_sampler());
        // m_desc_tracker->Bind(m_pass_info->render_resources->skybox_irradiance_cubemap.get(),
        //                      &cubemap_sampler,
        //                      Location(index++),
        //                      VK_SHADER_STAGE_FRAGMENT_BIT);
        // m_desc_tracker->Bind(m_pass_info->render_resources->skybox_prefilter_cubemap.get(),
        //                      &cubemap_sampler,
        //                      Location(index++),
        //                      VK_SHADER_STAGE_FRAGMENT_BIT);
        // m_desc_tracker->Bind(m_pass_info->render_resources->brdf_lut.get(),
        //                      &default_sampler,
        //                      Location(index++),
        //                      VK_SHADER_STAGE_FRAGMENT_BIT);
        // auto& frame_index = m_context->GetFrameIndex();
        // m_desc_tracker->Bind(m_framebuffers[frame_index].attachments[Shadowmap_Attachment].get(),
        //                      Location(index++),
        //                      VK_SHADER_STAGE_FRAGMENT_BIT);
        // m_desc_tracker->Sync();
    }

    std::optional<Shader> GraphicsPipelineShaders::GetShader(ShaderStage shader) { return m_shaders[shader]; }

    VkShaderStageFlagBits GraphicsPipelineShaders::ShaderStageToVkStage(ShaderStage stage) {
        static const std::unordered_map<ShaderStage, VkShaderStageFlagBits> umap {
            {Vertex_Shader, VK_SHADER_STAGE_VERTEX_BIT},
            {Tessellation_Control_Shader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
            {Tessellation_Evaluation_Shader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
            {Geometry_Shader, VK_SHADER_STAGE_GEOMETRY_BIT},
            {Fragment_Shader, VK_SHADER_STAGE_FRAGMENT_BIT},
        };

        return umap.at(stage);
    }

} // namespace Chandelier