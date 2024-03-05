#include "main_pass.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resource/asset_manager/asset_manager.h"
#include "runtime/core/base/exception.h"
#include "runtime/framework/global/global_context.h"
#include "render/base/common_vao_defines.h"
#include "render/base/render_pass_runner.h"
#include "precompute/brdf_lut.h"
#include "precompute/cubemap_prefilter.h"
#include "precompute/irradiance_convolution_pass.h"

#include "Buffer.h"
#include "Descriptor.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "VkContext.h"

#include "common_utils.h"

namespace Chandelier
{
#define MAIN_PASS_SETUP_CONTEXT \
    auto& context         = m_pass_info->render_context.vk_context; \
    auto& swapchain       = context->GetSwapchain(); \
    auto& command_manager = context->GetCommandManager();

    MainRenderPass::~MainRenderPass() { UnInit(); }

    void MainRenderPass::Recreate()
    {
        MAIN_PASS_SETUP_CONTEXT

        auto extent = swapchain.getExtent();

        m_pass_info->width  = extent.width;
        m_pass_info->height = extent.height;

        ResetFramebuffers();
        ResetPipeline();
        ResetAttachments();

        SetupAttachments();
        SyncDescriptorSets();
        SetupPipeline();
        SetupFramebuffers();
    }

    void MainRenderPass::UpdateUniformBuffer(const MainPassUniformBuffer& uniform_buffer)
    {
        /**
         * @todo: todo: map and unmap everytime or keep it mapped on memory?
         */
        m_ubo->map();
        m_ubo->Update(reinterpret_cast<const uint8_t*>(&uniform_buffer), sizeof(decltype(uniform_buffer)));
        m_ubo->Flush();
        m_ubo->unmap();
    }

    void MainRenderPass::Initialize(std::shared_ptr<BaseRenderPassInitInfo> info)
    {
        auto& context = info->render_context.vk_context;
        m_pass_info   = std::dynamic_pointer_cast<MainRenderPassInitInfo>(info);

        SetupUniformBuffer();
        SetupDescriptorSets();
        SetupAttachments();
        SetupPipeline();
        SetupFramebuffers();
    }

    void MainRenderPass::UnInit()
    {
        ResetFramebuffers();
        ResetPipeline();
        ResetAttachments();
        ResetDescriptorSets();
        ResetUniformBuffer();
    }

    void MainRenderPass::SetupUniformBuffer()
    {
        auto& context = m_pass_info->render_context.vk_context;

        m_ubo = std::make_shared<Buffer>();
        m_ubo->Allocate(context,
                        sizeof(MainPassUniformBuffer),
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    }

    void MainRenderPass::ResetUniformBuffer() { m_ubo = nullptr; }

    void MainRenderPass::SetupAttachments()
    {
        MAIN_PASS_SETUP_CONTEXT

        uint32_t width = m_pass_info->width, height = m_pass_info->height;
        size_t   frames_in_flight = context->GetSwapchain().getImageCount();

        /**
         * @todo: do we need to create all the attachments for each in fight frame?
         */
        m_framebuffers.resize(frames_in_flight);
        for (int i = 0; i < frames_in_flight; i++)
        {
            m_framebuffers[i].attachments.resize(Attachment_Max_Count);

            for (auto& attachment : m_framebuffers[i].attachments)
            {
                attachment = std::make_shared<Texture>();
            }

            auto& color_attachment         = m_framebuffers[i].attachments[Color_Attachment];
            auto& depth_stencil_attachment = m_framebuffers[i].attachments[DepthStencil_Attachment];

            bool enable_msaa = m_pass_info->memory_uniform_buffer->config.anti_aliasing == Enable_MSAA;

            color_attachment->InitAttachment(context,
                                             width,
                                             height,
                                             1,
                                             swapchain.getImageFormat(),
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                             enable_msaa);

            depth_stencil_attachment->InitAttachment(context,
                                                     width,
                                                     height,
                                                     1,
                                                     VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                                         VK_IMAGE_USAGE_SAMPLED_BIT,
                                                     enable_msaa);
        }
    }

    void MainRenderPass::ResetAttachments()
    {
        for (auto& framebuffer : m_framebuffers)
        {
            for (auto& attachment : framebuffer.attachments)
            {
                attachment = nullptr;
            }
        }
    }

    void MainRenderPass::SetupDescriptorSets()
    {
        MAIN_PASS_SETUP_CONTEXT
        
        m_desc_tracker = std::make_shared<DescriptorTracker>(context);
        SyncDescriptorSets();
    }

    void MainRenderPass::SyncDescriptorSets()
    {
        auto& context = m_pass_info->render_context.vk_context;

        auto& default_sampler = context->GetSampler(GPUSamplerState::default_sampler());
        
        size_t desc_loc = 0;
        m_desc_tracker->Bind(m_ubo.get(), Location(desc_loc++), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        
        for (auto&texture : m_pass_info->render_resources->model_tex_vec)
        {
            m_desc_tracker->Bind(texture.get(), &default_sampler, Location(desc_loc++), VK_SHADER_STAGE_FRAGMENT_BIT);
        }

        auto& cubemap_sampler = context->GetSampler(GPUSamplerState::cubemap_sampler());
        m_desc_tracker->Bind(m_pass_info->render_resources->skybox_prefilter_cubemap.get(),
                             &cubemap_sampler,
                             Location(desc_loc++),
                             VK_SHADER_STAGE_FRAGMENT_BIT);

        m_desc_tracker->Sync();
    }

    void MainRenderPass::ResetDescriptorSets()
    {
        m_desc_tracker = nullptr;
    }

    void MainRenderPass::SetupPipeline()
    {
        MAIN_PASS_SETUP_CONTEXT

        bool use_resolve = false;
        bool enable_msaa = m_pass_info->memory_uniform_buffer->config.anti_aliasing == Enable_MSAA;
        use_resolve |= enable_msaa;

        int attach_desc_count = use_resolve ? Attachment_Max_Count + 1 : Attachment_Max_Count;
        std::vector<VkAttachmentDescription> attachment_descs(attach_desc_count);
        {
            // color
            attachment_descs[Color_Attachment].format         = swapchain.getImageFormat();
            attachment_descs[Color_Attachment].samples =
                enable_msaa ? context->GetSuitableSampleCount() : VK_SAMPLE_COUNT_1_BIT;
            attachment_descs[Color_Attachment].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment_descs[Color_Attachment].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachment_descs[Color_Attachment].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment_descs[Color_Attachment].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment_descs[Color_Attachment].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment_descs[Color_Attachment].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            // depth
            attachment_descs[DepthStencil_Attachment].format         = VK_FORMAT_D32_SFLOAT_S8_UINT;
            attachment_descs[DepthStencil_Attachment].samples =
                enable_msaa ? context->GetSuitableSampleCount() : VK_SAMPLE_COUNT_1_BIT;
            attachment_descs[DepthStencil_Attachment].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment_descs[DepthStencil_Attachment].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachment_descs[DepthStencil_Attachment].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment_descs[DepthStencil_Attachment].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment_descs[DepthStencil_Attachment].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment_descs[DepthStencil_Attachment].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            if (use_resolve)
            {
                attachment_descs[Resolve_Attachment].format         = swapchain.getImageFormat();
                attachment_descs[Resolve_Attachment].samples        = VK_SAMPLE_COUNT_1_BIT;
                attachment_descs[Resolve_Attachment].loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment_descs[Resolve_Attachment].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
                attachment_descs[Resolve_Attachment].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment_descs[Resolve_Attachment].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachment_descs[Resolve_Attachment].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
                attachment_descs[Resolve_Attachment].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
        }
        
        std::vector<VkSubpassDescription> subpasses(Render_Pass_Count);
        
        VkAttachmentReference color_attach_ref {Color_Attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depth_attach_ref {DepthStencil_Attachment,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        VkAttachmentReference resolve_attach_ref {Resolve_Attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        subpasses[Main_Pass].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[Main_Pass].colorAttachmentCount    = 1;
        subpasses[Main_Pass].pColorAttachments       = &color_attach_ref;
        subpasses[Main_Pass].pDepthStencilAttachment = &depth_attach_ref;
        if (use_resolve)
            subpasses[Main_Pass].pResolveAttachments = &resolve_attach_ref;
        
        VkAttachmentReference skybox_attach_ref {Color_Attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        subpasses[Skybox_Pass].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[Skybox_Pass].colorAttachmentCount    = 1;
        subpasses[Skybox_Pass].pColorAttachments       = &skybox_attach_ref;
        subpasses[Skybox_Pass].pDepthStencilAttachment = &depth_attach_ref;
        if (use_resolve)
            subpasses[Skybox_Pass].pResolveAttachments = &resolve_attach_ref;

        /**
         * @todo: don't know how i wanna blend ui attachment and color attachment...just write ui pass on color attachment for now
         * maybe follow the piccolo solution, use ui pass as input attachment and run a combine ui pass
         */
        VkAttachmentReference ui_attach_ref {Color_Attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        subpasses[UI_Pass].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[UI_Pass].colorAttachmentCount    = 1;
        subpasses[UI_Pass].pColorAttachments       = &ui_attach_ref;
        if (use_resolve)
            subpasses[UI_Pass].pResolveAttachments = &resolve_attach_ref;

        /**
         * @todo: optimize the dependency setting, just to make it work for now 
         */
        std::vector<VkSubpassDependency> dependencies(2);
        dependencies[0] = {};
        dependencies[0].srcSubpass = Main_Pass;
        dependencies[0].dstSubpass = Skybox_Pass;
        dependencies[0].srcStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1]            = {};
        dependencies[1].srcSubpass = Skybox_Pass;
        dependencies[1].dstSubpass = UI_Pass;
        dependencies[1].srcStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].srcAccessMask   = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount        = attachment_descs.size();
        render_pass_info.pAttachments           = attachment_descs.data();
        render_pass_info.subpassCount           = subpasses.size();
        render_pass_info.pSubpasses             = subpasses.data();
        render_pass_info.dependencyCount        = dependencies.size();
        render_pass_info.pDependencies          = dependencies.data();
        VULKAN_API_CALL(vkCreateRenderPass(context->getDevice(), &render_pass_info, nullptr, &m_render_pipeline.render_pass));

        /**
         * @todo: implement the global path configurer and asset manager to eliminate the abs path
         */
        auto vert_shader = std::make_unique<Shader>();
        auto vert_code   = readBinaryFile("G:\\Visual Studio Projects\\VkEngineDemo\\engine\\shaders\\generated\\base_vert.spv");
        vert_shader->Initialize(context, reinterpret_cast<const uint8_t*>(vert_code.data()), vert_code.size());

        auto frag_shader = std::make_unique<Shader>();
        auto frag_code   = readBinaryFile("G:\\Visual Studio Projects\\VkEngineDemo\\engine\\shaders\\generated\\base_frag.spv");
        frag_shader->Initialize(context, reinterpret_cast<const uint8_t*>(frag_code.data()), frag_code.size());

        VkPipelineShaderStageCreateInfo vert_shader_info = {};
        vert_shader_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vert_shader_info.module = vert_shader->GetModule();
        vert_shader_info.pName  = "main";

        VkPipelineShaderStageCreateInfo frag_shader_info = {};
        frag_shader_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_info.module = frag_shader->GetModule();
        frag_shader_info.pName  = "main";

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {vert_shader_info, frag_shader_info};

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto vertex_binding_desc = ShaderData::Vertex::getBindingDescription();
        auto vert_attr_desc      = ShaderData::Vertex::getAttributeDescriptions();

        vertex_input_info.vertexBindingDescriptionCount   = 1;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vert_attr_desc.size());
        vertex_input_info.pVertexBindingDescriptions      = &vertex_binding_desc;
        vertex_input_info.pVertexAttributeDescriptions    = vert_attr_desc.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
        input_assembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        // NDC coordinates
        VkViewport viewport = {};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = 1.0;
        viewport.height   = 1.0;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = {1, 1};

        VkPipelineViewportStateCreateInfo viewport_state = {};
        viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports    = &viewport;
        viewport_state.scissorCount  = 1;
        viewport_state.pScissors     = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer  = {};
        rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable        = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth               = 1.0f;
        rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable         = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = (enable_msaa) ? context->GetSuitableSampleCount() : VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment = {};
        color_blend_attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable     = VK_FALSE;
        color_blending.logicOp           = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount   = 1;
        color_blending.pAttachments      = &color_blend_attachment;
        color_blending.blendConstants[0] = 0.0f;
        color_blending.blendConstants[1] = 0.0f;
        color_blending.blendConstants[2] = 0.0f;
        color_blending.blendConstants[3] = 0.0f;

        std::vector<VkDescriptorSetLayout> layouts{m_desc_tracker->GetSetLayout()};
        
        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount             = layouts.size();
        pipeline_layout_info.pSetLayouts                = layouts.data();

        VULKAN_API_CALL(
            vkCreatePipelineLayout(context->getDevice(), &pipeline_layout_info, nullptr, &m_render_pipeline.layout));

        std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
        dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
        pipeline_info.subpass                      = Main_Pass;
        pipeline_info.basePipelineHandle           = VK_NULL_HANDLE;

        VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
        depth_stencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable       = VK_TRUE;
        depth_stencil.depthWriteEnable      = VK_TRUE;
        depth_stencil.depthCompareOp        = VK_COMPARE_OP_LESS;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;

        pipeline_info.pDepthStencilState = &depth_stencil;

        VULKAN_API_CALL(vkCreateGraphicsPipelines(
            context->getDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_render_pipeline.pipeline));
    }

    void MainRenderPass::ResetPipeline()
    {
        auto& context = m_pass_info->render_context.vk_context;
        vkDestroyRenderPass(context->getDevice(), m_render_pipeline.render_pass, nullptr);
        vkDestroyPipeline(context->getDevice(), m_render_pipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(context->getDevice(), m_render_pipeline.layout, nullptr);
        
        m_render_pipeline.render_pass = VK_NULL_HANDLE;
        m_render_pipeline.pipeline    = VK_NULL_HANDLE;
        m_render_pipeline.layout      = VK_NULL_HANDLE;
    }

    void MainRenderPass::SetupFramebuffers()
    {
        MAIN_PASS_SETUP_CONTEXT

        size_t   frames_in_flight = context->GetSwapchain().getImageCount();
        uint32_t width = m_pass_info->width, height = m_pass_info->height;
        auto swapchain_extent = context->GetSwapchain().getExtent();

        m_framebuffers.resize(frames_in_flight);
        for (size_t i = 0; i < frames_in_flight; i++)
        {
            m_framebuffers[i].render_pass = m_render_pipeline.render_pass;
            m_framebuffers[i].render_area = {{0, 0}, {width, height}};

            auto& color_attachment         = m_framebuffers[i].attachments[Color_Attachment];
            auto& depth_stencil_attachment = m_framebuffers[i].attachments[DepthStencil_Attachment];
            
            bool enable_msaa = m_pass_info->memory_uniform_buffer->config.anti_aliasing == Enable_MSAA;
            
            std::vector<VkImageView> attachments;
            if (enable_msaa)
            {
                attachments = {color_attachment->getView(), depth_stencil_attachment->getView(), swapchain.getImageView(i)};
            }
            else
            {
                attachments = {swapchain.getImageView(i), depth_stencil_attachment->getView()};
            }
            
            VkFramebufferCreateInfo framebuffer_create_info = {};
            framebuffer_create_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass              = m_framebuffers[i].render_pass;
            framebuffer_create_info.attachmentCount         = attachments.size();
            framebuffer_create_info.pAttachments            = attachments.data();
            framebuffer_create_info.width                   = swapchain_extent.width;
            framebuffer_create_info.height                  = swapchain_extent.height;
            framebuffer_create_info.layers                  = 1;

            VULKAN_API_CALL(vkCreateFramebuffer(
                context->getDevice(), &framebuffer_create_info, nullptr, &m_framebuffers[i].handle));
        }
    }

    void MainRenderPass::ResetFramebuffers()
    {
        auto& context = m_pass_info->render_context.vk_context;

        for (auto frambuffer : m_framebuffers)
        {
            vkDestroyFramebuffer(context->getDevice(), frambuffer.handle, nullptr);
            frambuffer.handle = VK_NULL_HANDLE;
        }

        m_framebuffers.clear();
    }

    void MainRenderPass::PreDrawSetup()
    {
// #define COMPUTE_BRDF_LUT
#define COMPUTE_PREFILTER
#define COMPUTE_IRRADIANCE

        MAIN_PASS_SETUP_CONTEXT

        // @todo: file check
        RenderPassRunner runner;

#ifdef COMPUTE_BRDF_LUT
        {
            // ibl brdf lut
            auto brdflut_init_info              = std::make_shared<BRDFLutInitInfo>();
            brdflut_init_info->width            = 512;
            brdflut_init_info->height           = 512;
            brdflut_init_info->render_context   = m_pass_info->render_context;
            brdflut_init_info->render_resources = m_pass_info->render_resources;

            auto brdf_lut_pass = std::make_shared<BRDFLutPass>();
            brdf_lut_pass->Initialize(brdflut_init_info);
            runner.Initialize(brdf_lut_pass);
            runner.Run();
            runner.Save("brdf_lut.png");
        }
#endif

        const glm::mat4 vulkanCorrection = {
            {1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f, 1.0f}};
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[]    = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // X+
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // X-
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), // Z+
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), // Z-
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // Y+
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // Y-
        };

        {
            // ibl cubemap prefilter
            auto cubemap_prefilter_init_info              = std::make_shared<CubeMapPrefilterInitInfo>();
            cubemap_prefilter_init_info->width            = CUBEMAP_PREFILTER_BASE_WIDTH;
            cubemap_prefilter_init_info->height           = CUBEMAP_PREFILTER_BASE_HEIGHT;
            cubemap_prefilter_init_info->render_context   = m_pass_info->render_context;
            cubemap_prefilter_init_info->render_resources = m_pass_info->render_resources;
        
            auto cubemap_prefilter_pass = std::make_shared<CubeMapPrefilterPass>();
            cubemap_prefilter_pass->Initialize(cubemap_prefilter_init_info);
            runner.Initialize(cubemap_prefilter_pass);
            
            for (int mip_level = 0; mip_level < CUBEMAP_PREFILTER_MIP_LEVEL; mip_level++)
            {
                CubemapFilterPassUniformBuffer prefilter_ubo;
                prefilter_ubo.roughness = (float)mip_level / (CUBEMAP_PREFILTER_MIP_LEVEL - 1);

                uint32_t data_width     = CUBEMAP_PREFILTER_BASE_WIDTH * std::pow(0.5, mip_level);
                uint32_t data_height    = CUBEMAP_PREFILTER_BASE_HEIGHT * std::pow(0.5, mip_level);
                cubemap_prefilter_init_info->width  = data_width;
                cubemap_prefilter_init_info->height = data_height;

                for (int layer = 0; layer < 6; ++layer)
                {
                    prefilter_ubo.view       = captureViews[layer];
                    prefilter_ubo.projection = vulkanCorrection * captureProjection;

                    cubemap_prefilter_pass->UpdateUniformBuffer(prefilter_ubo);
                    runner.Run();
                    std::string filename = "cubemap_prefilter_mip_" + std::to_string(mip_level) + "_layer_" +
                                           std::to_string(layer) + ".png";
                    // runner.Save(filename);

                    auto prefilter_attachment = cubemap_prefilter_pass->GetAttachment(0, 0);
                    prefilter_attachment->TransferLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                    
                    VkImageCopy copy_region = {};
                    
                    copy_region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                    copy_region.srcSubresource.baseArrayLayer = 0;
                    copy_region.srcSubresource.mipLevel       = 0;
                    copy_region.srcSubresource.layerCount     = 1;
                    copy_region.srcOffset                     = {0, 0, 0};
                    
                    copy_region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                    copy_region.dstSubresource.baseArrayLayer = layer;
                    copy_region.dstSubresource.mipLevel       = mip_level;
                    copy_region.dstSubresource.layerCount     = 1;
                    copy_region.dstOffset                     = {0, 0, 0};
                    
                    copy_region.extent.width  = static_cast<uint32_t>(data_width);
                    copy_region.extent.height = static_cast<uint32_t>(data_height);
                    copy_region.extent.depth  = 1;
                    
                    command_manager.Copy(prefilter_attachment.get(),
                                         m_pass_info->render_resources->skybox_prefilter_cubemap.get(),
                                         std::vector<VkImageCopy> {copy_region});
                    
                    prefilter_attachment->TransferLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                    prefilter_attachment->TransferLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                    SaveTexture(m_pass_info->render_resources->skybox_prefilter_cubemap, filename, layer, mip_level);
                }
            }
            // command_manager.Submit();
        }

        #ifdef COMPUTE_IRRADIANCE
        {
            // ibl irradiance convolution
            auto irradiance_init_info              = std::make_shared<IrradianceConvolutionPassInitInfo>();
            irradiance_init_info->width            = 256;
            irradiance_init_info->height           = 256;
            irradiance_init_info->render_context   = m_pass_info->render_context;
            irradiance_init_info->render_resources = m_pass_info->render_resources;

            auto irradiance_convolution_pass = std::make_shared<IrradianceConvolutionPass>();
            irradiance_convolution_pass->Initialize(irradiance_init_info);
            runner.Initialize(irradiance_convolution_pass);

            for (unsigned int layer = 0; layer < 6; ++layer)
            {
                IrradianceConvolutionPassUniformBuffer prefilter_ubo;
                prefilter_ubo.view       = captureViews[layer];
                prefilter_ubo.projection = vulkanCorrection * captureProjection;

                irradiance_convolution_pass->UpdateUniformBuffer(prefilter_ubo);

                runner.Run();
                // std::string filename = "irradiance_map_" + std::to_string(i) + ".png";
                // runner.Save(filename);

                auto irradiance_attachment = irradiance_convolution_pass->GetAttachment(0, 0);
                irradiance_attachment->TransferLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

                VkImageCopy copy_region = {};

                copy_region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                copy_region.srcSubresource.baseArrayLayer = 0;
                copy_region.srcSubresource.mipLevel       = 0;
                copy_region.srcSubresource.layerCount     = 1;
                copy_region.srcOffset                     = {0, 0, 0};

                copy_region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                copy_region.dstSubresource.baseArrayLayer = layer;
                copy_region.dstSubresource.mipLevel       = 0;
                copy_region.dstSubresource.layerCount     = 1;
                copy_region.dstOffset                     = {0, 0, 0};

                copy_region.extent.width  = static_cast<uint32_t>(irradiance_init_info->width);
                copy_region.extent.height = static_cast<uint32_t>(irradiance_init_info->height);
                copy_region.extent.depth  = 1;

                command_manager.Copy(irradiance_attachment.get(),
                                     m_pass_info->render_resources->skybox_irradiance_cubemap.get(),
                                     std::vector<VkImageCopy> {copy_region});

                irradiance_attachment->TransferLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            command_manager.Submit();
        }
        #endif
    }

    void MainRenderPass::PostDrawCallback()
    {
        MAIN_PASS_SETUP_CONTEXT 

        // color_attachment->TransferLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    void MainRenderPass::Draw()
    {
        MAIN_PASS_SETUP_CONTEXT 

        // todo: is it necessary to bind everytime before drawing? if not relocate it later
        for (auto& mesh : m_pass_info->render_resources->model_mesh_vec)
        {
            auto                      index_buffer       = mesh->GetBuffer(Index_Buffer);
            std::vector<VkBuffer>     bind_index_buffers = {index_buffer};
            std::vector<VkDeviceSize> bind_index_offsets = {0};
            command_manager.Bind(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, bind_index_buffers, bind_index_offsets);
            
            auto                      vertex_buffer            = mesh->GetBuffer(Vertex_Buffer);
            auto                      vertex_count             = mesh->getVertexCount();
            std::vector<VkBuffer>     bind_vertex_buffers      = {vertex_buffer, vertex_buffer, vertex_buffer, vertex_buffer};
            std::vector<VkDeviceSize> bind_vertex_attr_offsets = {0,
                                                                  vertex_count * sizeof(glm::vec3),
                                                                  vertex_count * sizeof(glm::vec3) * 2,
                                                                  vertex_count * sizeof(glm::vec3) * 3};
            command_manager.Bind(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bind_vertex_buffers, bind_vertex_attr_offsets);
            
            command_manager.DrawIndexed(mesh.get());
        }

        PostDrawCallback();
    }

    void MainRenderPass::ForwardDraw(std::vector<std::shared_ptr<RenderPass>> subpasses)
    {
        MAIN_PASS_SETUP_CONTEXT
        uint32_t width = m_pass_info->width, height = m_pass_info->height;
        auto&    frame_index        = context->GetFrameIndex();
        auto&    active_framebuffer = m_framebuffers[frame_index];

        command_manager.ActivateFramebuffer(active_framebuffer);

        command_manager.BeginRenderPass(active_framebuffer);
        command_manager.BindPipeline(m_render_pipeline.pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);

        m_desc_tracker->BindDescriptorSet(m_render_pipeline.layout, VK_PIPELINE_BIND_POINT_GRAPHICS);

        command_manager.SetViewport(VkViewport {0.f, 0.f, (float)width, (float)height, 0.f, 1.f});
        command_manager.SetScissor(VkRect2D {{0, 0}, {width, height}});

        {
            this->Draw();

            for (auto& subpass : subpasses)
            {
                command_manager.NextSubpass();
                subpass->Draw();
            }
        }

        command_manager.EndRenderPass(active_framebuffer);
        command_manager.Submit();
    }

    const VkRenderPass* MainRenderPass::GetRenderPass() { return &m_render_pipeline.render_pass;}

} // namespace Chandelier