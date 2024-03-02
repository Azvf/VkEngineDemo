#include "brdf_lut.h"

#include "runtime/core/base/exception.h"
#include "render/base/common_vao_defines.h"

#include "Shader.h"
#include "Texture.h"
#include "VkContext.h"
#include "Mesh.h"

#include "common_utils.h"

namespace Chandelier
{
#define BRDF_LUT_PASS_SETUP_CONTEXT \
    auto& context         = m_pass_info->render_context.vk_context; \
    auto& command_manager = context->GetCommandManager();

#define BRDF_LUT_FORMAT VK_FORMAT_R8G8B8A8_UNORM

    BRDFLutPass::~BRDFLutPass() { UnInit(); }

    void BRDFLutPass::Initialize(std::shared_ptr<BaseRenderPassInitInfo> info)
    {
        m_pass_info = std::dynamic_pointer_cast<BRDFLutInitInfo>(info);
        auto& context  = m_pass_info->render_context.vk_context;
        
        m_screen_mesh = Mesh::load(context, Has_UV, ScreenVertices.data(), ScreenVertices.size(), nullptr, 0);

        SetupAttachments();
        SetupPipeline();
        SetupFramebuffers();
    }

    void BRDFLutPass::UnInit()
    {
        ResetFramebuffers();
        ResetPipeline();
        ResetAttachments();
    }

    void BRDFLutPass::Recreate() {}

    const VkRenderPass* BRDFLutPass::GetRenderPass() { return &m_render_pipeline.render_pass; }

    void BRDFLutPass::SetupAttachments()
    {
        BRDF_LUT_PASS_SETUP_CONTEXT

        uint32_t width = m_pass_info->width, height = m_pass_info->height;

        m_framebuffers.resize(1);
        auto& framebuffer = m_framebuffers.front();
        framebuffer.attachments.resize(1);
        auto& attachment = framebuffer.attachments.front();

        attachment = std::make_shared<Texture>();
        attachment->InitAttachment(context,
                                   width,
                                   height,
                                   BRDF_LUT_FORMAT,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    }

    void BRDFLutPass::ResetAttachments()
    {
        for (auto& framebuffer : m_framebuffers)
        {
            for (auto& attachment : framebuffer.attachments)
            {
                attachment = nullptr;
            }
        }
    }

    void BRDFLutPass::SetupPipeline()
    {
        BRDF_LUT_PASS_SETUP_CONTEXT

        std::vector<VkAttachmentDescription> attachment_descs(1);
        // color
        attachment_descs[0].format         = BRDF_LUT_FORMAT;
        attachment_descs[0].samples        = VK_SAMPLE_COUNT_1_BIT;
        attachment_descs[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descs[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descs[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descs[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        std::vector<VkSubpassDescription> subpasses(1);
        VkAttachmentReference             color_attach_ref {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        subpasses[0].pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[0].colorAttachmentCount = 1;
        subpasses[0].pColorAttachments    = &color_attach_ref;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount        = attachment_descs.size();
        render_pass_info.pAttachments           = attachment_descs.data();
        render_pass_info.subpassCount           = subpasses.size();
        render_pass_info.pSubpasses             = subpasses.data();
        render_pass_info.dependencyCount        = 0;
        render_pass_info.pDependencies          = nullptr;
        VULKAN_API_CALL(
            vkCreateRenderPass(context->getDevice(), &render_pass_info, nullptr, &m_render_pipeline.render_pass));

        auto vert_shader = std::make_unique<Shader>();
        auto vert_code =
            readBinaryFile("G:\\Visual Studio Projects\\VkEngineDemo\\engine\\shaders\\generated\\brdf_lut_gen_vert.spv");
        vert_shader->Initialize(context, reinterpret_cast<const uint8_t*>(vert_code.data()), vert_code.size());

        auto frag_shader = std::make_unique<Shader>();
        auto frag_code =
            readBinaryFile("G:\\Visual Studio Projects\\VkEngineDemo\\engine\\shaders\\generated\\brdf_lut_gen_frag.spv");
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

        std::vector<VkVertexInputBindingDescription> vert_binding_descs = m_screen_mesh->GetBindingDescription();
        std::vector<VkVertexInputAttributeDescription> vert_attr_descs = m_screen_mesh->GetAttributeDescriptions();
        
        vertex_input_info.vertexBindingDescriptionCount   = vert_binding_descs.size();
        vertex_input_info.pVertexBindingDescriptions      = vert_binding_descs.data();
        vertex_input_info.vertexAttributeDescriptionCount = vert_attr_descs.size();
        vertex_input_info.pVertexAttributeDescriptions    = vert_attr_descs.data();

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
        viewport.maxDepth   = 1.0f;

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
        multisampling.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;

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

        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount             = 0;
        pipeline_layout_info.pSetLayouts                = nullptr;

        VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
        depth_stencil.sType           = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable  = VK_TRUE;
        depth_stencil.depthWriteEnable                      = VK_TRUE;
        depth_stencil.depthCompareOp                        = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_stencil.depthBoundsTestEnable                 = VK_FALSE;

        VULKAN_API_CALL(
            vkCreatePipelineLayout(context->getDevice(), &pipeline_layout_info, nullptr, &m_render_pipeline.layout));

        std::vector<VkDynamicState>      dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
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
        pipeline_info.subpass                      = 0;
        pipeline_info.basePipelineHandle           = VK_NULL_HANDLE;
        pipeline_info.pDepthStencilState           = &depth_stencil;

        VULKAN_API_CALL(vkCreateGraphicsPipelines(
            context->getDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_render_pipeline.pipeline));
    }

    void BRDFLutPass::ResetPipeline()
    {
        auto& context = m_pass_info->render_context.vk_context;

        vkDestroyRenderPass(context->getDevice(), m_render_pipeline.render_pass, nullptr);
        vkDestroyPipeline(context->getDevice(), m_render_pipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(context->getDevice(), m_render_pipeline.layout, nullptr);

        m_render_pipeline.render_pass = VK_NULL_HANDLE;
        m_render_pipeline.pipeline    = VK_NULL_HANDLE;
        m_render_pipeline.layout      = VK_NULL_HANDLE;
    }

    void BRDFLutPass::SetupFramebuffers()
    {
        BRDF_LUT_PASS_SETUP_CONTEXT

        auto& framebuffer = m_framebuffers.front();
        auto& attachment  = framebuffer.attachments.front();

        uint32_t width = m_pass_info->width, height = m_pass_info->height;

        framebuffer.render_pass = m_render_pipeline.render_pass;
        framebuffer.render_area = {{0, 0}, {width, height}};

        std::vector<VkImageView> attachments {attachment->getView()};

        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass              = framebuffer.render_pass;
        framebuffer_create_info.attachmentCount         = attachments.size();
        framebuffer_create_info.pAttachments            = attachments.data();
        framebuffer_create_info.width                   = width;
        framebuffer_create_info.height                  = height;
        framebuffer_create_info.layers                  = 1;

        VULKAN_API_CALL(
            vkCreateFramebuffer(context->getDevice(), &framebuffer_create_info, nullptr, &framebuffer.handle));
    }

    void BRDFLutPass::ResetFramebuffers()
    {
        auto& context = m_pass_info->render_context.vk_context;

        for (auto frambuffer : m_framebuffers)
        {
            vkDestroyFramebuffer(context->getDevice(), frambuffer.handle, nullptr);
            frambuffer.handle = VK_NULL_HANDLE;
        }

        m_framebuffers.clear();
    }

    void BRDFLutPass::PreDrawSetup() {}

    void BRDFLutPass::Draw()
    {
        BRDF_LUT_PASS_SETUP_CONTEXT
        uint32_t width = m_pass_info->width, height = m_pass_info->height;

        auto& active_framebuffer = m_framebuffers.front();

        command_manager.ActivateFramebuffer(active_framebuffer);

        command_manager.BeginRenderPass(active_framebuffer);
        command_manager.BindPipeline(m_render_pipeline.pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);

        command_manager.SetViewport(VkViewport {0.f, 0.f, (float)width, (float)height, 0.f, 1.f});
        command_manager.SetScissor(VkRect2D {{0, 0}, {width, height}});

        auto                      vertex_buffer            = m_screen_mesh->GetBuffer(Vertex_Buffer);
        auto                      vertex_count             = m_screen_mesh->getVertexCount();
        std::vector<VkBuffer>     bind_vertex_buffers      = {vertex_buffer, vertex_buffer};
        std::vector<VkDeviceSize> bind_vertex_attr_offsets = {0, vertex_count * sizeof(float) * 3};
        command_manager.Bind(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bind_vertex_buffers, bind_vertex_attr_offsets);
        command_manager.Draw(vertex_count, 1);

        command_manager.EndRenderPass(active_framebuffer);
        command_manager.Submit();
    }

    void BRDFLutPass::PostDrawCallback() {}

} // namespace Chandelier