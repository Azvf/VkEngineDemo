#include "skybox_pass.h"

#include "resource/asset_manager/asset_manager.h"
#include "runtime/core/base/exception.h"

#include "main_pass.h"
#include "VkContext.h"
#include "Descriptor.h"
#include "Shader.h"
#include "Mesh.h"
#include "Buffer.h"

#include "common_utils.h"

namespace Chandelier
{
#define SKYBOX_PASS_SETUP_CONTEXT \
    auto& context         = m_pass_info->render_context.vk_context; \
    auto& swapchain       = context->GetSwapchain(); \
    auto& command_manager = context->GetCommandManager();

    SkyboxPass::~SkyboxPass() { UnInit(); }
    
    const VkRenderPass* SkyboxPass::GetRenderPass() { return m_pass_info->render_pass; }

    void SkyboxPass::Initialize(std::shared_ptr<BaseRenderPassInitInfo> info)
    {
        m_pass_info = std::dynamic_pointer_cast<SkyboxPassInitInfo>(info);
        m_render_pipeline.render_pass = *m_pass_info->render_pass;

        SetupShaderResources();
        SetupDescriptorSets();
        SetupPipeline();
    }

    void SkyboxPass::UnInit()
    {
        ResetPipeline();
        ResetDescriptorSets();
        ResetShaderResources();
    }

    void SkyboxPass::Recreate() { 
        m_render_pipeline.render_pass = *m_pass_info->render_pass;
        assert(m_render_pipeline.render_pass);

        ResetPipeline();
        SetupPipeline();
    }

    void SkyboxPass::SetupShaderResources()
    {
        SKYBOX_PASS_SETUP_CONTEXT
        std::array<std::shared_ptr<Texture>, 6> skybox_faces;
        skybox_faces[0] = LoadTextureHDR(
            context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_X+.hdr", 4);
        skybox_faces[1] = LoadTextureHDR(
            context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_X-.hdr", 4);
        skybox_faces[2] = LoadTextureHDR(
            context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_Z+.hdr", 4);
        skybox_faces[3] = LoadTextureHDR(
            context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_Z-.hdr", 4);
        skybox_faces[4] = LoadTextureHDR(
            context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_Y+.hdr", 4);
        skybox_faces[5] = LoadTextureHDR(
            context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_Y-.hdr", 4);
        m_skybox_tex = LoadSkybox(context, skybox_faces, 4);

        m_ubo = std::make_shared<Buffer>();
        m_ubo->Allocate(context,
                        sizeof(SkyboxPassUniformBuffer),
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    }

    void SkyboxPass::ResetShaderResources()
    {
        m_skybox_tex = nullptr;
        m_ubo        = nullptr;
    }

    void SkyboxPass::UpdateUniformBuffer(const SkyboxPassUniformBuffer& uniform_buffer)
    {
        m_ubo->map();
        m_ubo->Update(reinterpret_cast<const uint8_t*>(&uniform_buffer), sizeof(decltype(uniform_buffer)));
        m_ubo->Flush();
        m_ubo->unmap();
    }

    void SkyboxPass::SetupPipeline() { 
        SKYBOX_PASS_SETUP_CONTEXT
        bool enable_msaa = m_pass_info->main_pass_uniform_buffer->config.anti_aliasing == Enable_MSAA;
 
        auto vert_shader = std::make_unique<Shader>();
        auto vert_code   = readBinaryFile("G:\\Visual Studio Projects\\VkEngineDemo\\engine\\shaders\\generated\\skybox_vert.spv");
        vert_shader->Initialize(context, reinterpret_cast<const uint8_t*>(vert_code.data()), vert_code.size());

        auto frag_shader = std::make_unique<Shader>();
        auto frag_code   = readBinaryFile("G:\\Visual Studio Projects\\VkEngineDemo\\engine\\shaders\\generated\\skybox_frag.spv");
        frag_shader->Initialize(context, reinterpret_cast<const uint8_t*>(frag_code.data()), frag_code.size());

        VkPipelineShaderStageCreateInfo vert_shader_info = {};
        vert_shader_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_info.stage                           = VK_SHADER_STAGE_VERTEX_BIT;
        vert_shader_info.module                          = vert_shader->GetModule();
        vert_shader_info.pName                           = "main";

        VkPipelineShaderStageCreateInfo frag_shader_info = {};
        frag_shader_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_info.stage                           = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_info.module                          = frag_shader->GetModule();
        frag_shader_info.pName                           = "main";

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {vert_shader_info, frag_shader_info};

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        // auto vertex_binding_desc = ShaderData::Vertex::getBindingDescription();
        // auto vert_attr_desc      = ShaderData::Vertex::getAttributeDescriptions();
        // 
        // vertex_input_info.vertexBindingDescriptionCount   = 1;
        // vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vert_attr_desc.size());
        // vertex_input_info.pVertexBindingDescriptions      = &vertex_binding_desc;
        // vertex_input_info.pVertexAttributeDescriptions    = vert_attr_desc.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
        input_assembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        // NDC coordinates
        VkViewport viewport = {};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = 1.0;
        viewport.height     = 1.0;
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 0.0f;

        VkRect2D scissor = {};
        scissor.offset   = {0, 0};
        scissor.extent   = {1, 1};

        VkPipelineViewportStateCreateInfo viewport_state = {};
        viewport_state.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount                     = 1;
        viewport_state.pViewports                        = &viewport;
        viewport_state.scissorCount                      = 1;
        viewport_state.pScissors                         = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable                       = VK_FALSE;
        rasterizer.rasterizerDiscardEnable                = VK_FALSE;
        rasterizer.polygonMode                            = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth                              = 1.0f;
        rasterizer.cullMode                               = /*VK_CULL_MODE_BACK_BIT*/ VK_CULL_MODE_NONE;
        rasterizer.frontFace                              = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable                        = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = (enable_msaa) ? context->GetSuitableSampleCount() : VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment = {};
        color_blend_attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable                       = VK_FALSE;
        color_blending.logicOp                             = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount                     = 1;
        color_blending.pAttachments                        = &color_blend_attachment;
        color_blending.blendConstants[0]                   = 0.0f;
        color_blending.blendConstants[1]                   = 0.0f;
        color_blending.blendConstants[2]                   = 0.0f;
        color_blending.blendConstants[3]                   = 0.0f;

        std::vector<VkDescriptorSetLayout> layouts {m_desc_tracker->GetSetLayout()};

        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount             = layouts.size();
        pipeline_layout_info.pSetLayouts                = layouts.data();

        VULKAN_API_CALL(
            vkCreatePipelineLayout(context->getDevice(), &pipeline_layout_info, nullptr, &m_render_pipeline.layout));

        std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
        dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
        dynamic_state_create_info.pDynamicStates    = dynamic_states.data();

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount                   = shader_stages.size();
        pipeline_info.pStages                      = shader_stages.data();
        pipeline_info.pVertexInputState            = &vertex_input_info;
        pipeline_info.pInputAssemblyState          = &input_assembly;
        pipeline_info.pViewportState               = &viewport_state;
        pipeline_info.pRasterizationState          = &rasterizer;
        pipeline_info.pMultisampleState            = &multisampling;
        pipeline_info.pColorBlendState             = &color_blending;
        pipeline_info.pDynamicState                = &dynamic_state_create_info;
        pipeline_info.layout                       = m_render_pipeline.layout;
        pipeline_info.renderPass                   = m_render_pipeline.render_pass;
        pipeline_info.subpass                      = Skybox_Pass;
        pipeline_info.basePipelineHandle           = VK_NULL_HANDLE;

        VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
        depth_stencil.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable                       = VK_TRUE;
        depth_stencil.depthWriteEnable                      = VK_TRUE;
        depth_stencil.depthCompareOp                        = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_stencil.depthBoundsTestEnable                 = VK_FALSE;

        pipeline_info.pDepthStencilState = &depth_stencil;

        VULKAN_API_CALL(vkCreateGraphicsPipelines(
            context->getDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_render_pipeline.pipeline));

    }
    
    void SkyboxPass::ResetPipeline() { 
        SKYBOX_PASS_SETUP_CONTEXT
        
        vkDestroyPipeline(context->getDevice(), m_render_pipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(context->getDevice(), m_render_pipeline.layout, nullptr);

        m_render_pipeline.pipeline    = VK_NULL_HANDLE;
        m_render_pipeline.layout      = VK_NULL_HANDLE;
    }

    void SkyboxPass::SetupDescriptorSets()
    {
        SKYBOX_PASS_SETUP_CONTEXT
        m_desc_tracker = std::make_shared<DescriptorTracker>(context);
        SyncDescriptorSets();
    }

    void SkyboxPass::SyncDescriptorSets()
    {
        SKYBOX_PASS_SETUP_CONTEXT
        auto& cubemap_sampler = context->GetSampler(GPUSamplerState::cubemap_sampler());
        m_desc_tracker->Bind(m_ubo.get(), Location(0), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        m_desc_tracker->Bind(m_skybox_tex.get(), &cubemap_sampler, Location(1), VK_SHADER_STAGE_FRAGMENT_BIT);
        m_desc_tracker->Sync();
    }

    void SkyboxPass::ResetDescriptorSets() { m_desc_tracker = nullptr; }

    void SkyboxPass::PreDrawSetup() {}
    void SkyboxPass::PostDrawCallback() {}
    void SkyboxPass::Draw()
    {
        auto& main_pass_ubo = m_pass_info->main_pass_uniform_buffer;

        if (!main_pass_ubo->config.show_skybox)
            return;

        SKYBOX_PASS_SETUP_CONTEXT

        command_manager.BindPipeline(m_render_pipeline.pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
        m_desc_tracker->BindDescriptorSet(m_render_pipeline.layout, VK_PIPELINE_BIND_POINT_GRAPHICS);

        command_manager.Draw(3, 1);
    }


} // namespace Chandelier