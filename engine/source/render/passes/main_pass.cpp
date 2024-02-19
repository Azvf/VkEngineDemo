#include "main_pass.h"

#include "resource/asset_manager/asset_manager.h"
#include "runtime/core/base/exception.h"
#include "runtime/framework/global/global_context.h"

#include "Buffer.h"
#include "Descriptor.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "VkContext.h"

#include "common_utils.h"

namespace Chandelier
{
    MainRenderPass::~MainRenderPass() { UnInit(); }

    void MainRenderPass::Resize(size_t width, size_t height)
    {
        if (!m_pass_info)
        {
            assert(0);
            return;
        }

        m_pass_info->width  = width;
        m_pass_info->height = height;

        ResetSwapchainFramebuffer();
        ResetAttachments();

        SetupAttachments();
        SyncDescriptorSets();
        SetupSwapchainFramebuffer();
    }

    void MainRenderPass::UpdateUniformBuffer(const MainPassUniformBuffer& uniform_buffer)
    {
        auto& context         = m_pass_info->render_context.vk_context;
        auto& command_buffers = context->GetCommandBuffers();

        // todo: map and unmap everytime or keep it mapped on memory
        m_ubo->map();
        m_ubo->Update(reinterpret_cast<const uint8_t*>(&uniform_buffer), sizeof(MainPassUniformBuffer));
        m_ubo->Flush();
        m_ubo->unmap();
    }

    void MainRenderPass::Initialize(std::shared_ptr<BaseRenderPassInitInfo> info)
    {
        auto& context = info->render_context.vk_context;
        m_pass_info   = std::dynamic_pointer_cast<MainRenderPassInitInfo>(info);

        m_meshes.push_back(LoadObjModel(context, "G:\\Visual Studio Projects\\VkEngineDemo\\engine\\assets\\Boat\\Boat.obj"));
        m_textures.push_back(LoadTexture(context, "G:\\Visual Studio Projects\\VkEngineDemo\\engine\\assets\\Boat\\texture\\bench 1_Base_color.png"));
        m_textures.push_back(LoadTexture(context, "G:\\Visual Studio Projects\\VkEngineDemo\\engine\\assets\\Boat\\texture\\bench 1_Normal.png"));

        SetupUniformBuffer();
        SetupDescriptorSets();
        SetupAttachments();
        SetupPipeline();
        SetupSwapchainFramebuffer();
    }

    void MainRenderPass::UnInit()
    {
        ResetSwapchainFramebuffer();
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
                        sizeof(MainPassUniformBuffer), // todo: op away magic number
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    }

    void MainRenderPass::ResetUniformBuffer() {}

    void MainRenderPass::SetupAttachments()
    {
        auto&    context = m_pass_info->render_context.vk_context;
        uint32_t width = m_pass_info->width, height = m_pass_info->height;
        size_t   frames_in_flight = context->GetSwapchain().getImageCount();
        m_framebuffers.resize(frames_in_flight);

        for (int i = 0; i < frames_in_flight; i++)
        {
            m_framebuffers[i].render_area = {{0, 0}, {width, height}};
            m_framebuffers[i].attachments.resize(Attachment_Max_Count);

            for (auto& attachment : m_framebuffers[i].attachments)
            {
                attachment = std::make_shared<Texture>();
            }

            auto& color_attachment         = m_framebuffers[i].attachments[Color_Attachment];
            auto& depth_stencil_attachment = m_framebuffers[i].attachments[DepthStencil_Attachment];

            color_attachment->InitTex2D(context,
                                        width,
                                        height,
                                        1,
                                        1,
                                        VK_FORMAT_R16G16B16A16_SFLOAT,
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            depth_stencil_attachment->InitTex2D(context,
                                                width,
                                                height,
                                                1,
                                                1,
                                                VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                                    VK_IMAGE_USAGE_SAMPLED_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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
        auto& context = m_pass_info->render_context.vk_context;

        // for (auto& desc_tracker : m_desc_array)
        // {
        //     desc_tracker = std::make_shared<DescriptorTracker>(context);
        // }

        m_desc_tracker = std::make_shared<DescriptorTracker>(context);
        SyncDescriptorSets();
    }

    void MainRenderPass::SyncDescriptorSets()
    {
        auto& context = m_pass_info->render_context.vk_context;

        // m_desc_array[Global_Mesh_Layout]->Bind(
        //     m_ubo.get(), Location(0), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        // m_desc_array[Global_Mesh_Layout]->Sync();
        // 
        // auto& default_sampler = context->GetSampler(GPUSamplerState::default_sampler());
        // m_desc_array[Mesh_Material_Layout]->Bind(
        //     m_textures[0].get(), &default_sampler, Location(0), VK_SHADER_STAGE_FRAGMENT_BIT);
        // m_desc_array[Mesh_Material_Layout]->Bind(
        //     m_textures[1].get(), &default_sampler, Location(1), VK_SHADER_STAGE_FRAGMENT_BIT);
        // m_desc_array[Mesh_Material_Layout]->Sync();
    
        auto& default_sampler = context->GetSampler(GPUSamplerState::default_sampler());
        
        m_desc_tracker->Bind(m_ubo.get(), Location(0), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        m_desc_tracker->Bind(m_textures[0].get(), &default_sampler, Location(1), VK_SHADER_STAGE_FRAGMENT_BIT);
        m_desc_tracker->Bind(m_textures[1].get(), &default_sampler, Location(2), VK_SHADER_STAGE_FRAGMENT_BIT);
        m_desc_tracker->Sync();
    }

    void MainRenderPass::ResetDescriptorSets()
    {
        // for (auto& desc_tracker : m_desc_array)
        // {
        //     desc_tracker = nullptr;
        // }
        m_desc_tracker = nullptr;
    }

    void MainRenderPass::SetupPipeline()
    {
        auto& context = m_pass_info->render_context.vk_context;

        VkAttachmentDescription attachment_descs[Attachment_Max_Count] = {};
        {
            // color
            attachment_descs[Color_Attachment].format         = VK_FORMAT_R16G16B16A16_SFLOAT;
            attachment_descs[Color_Attachment].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachment_descs[Color_Attachment].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment_descs[Color_Attachment].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachment_descs[Color_Attachment].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment_descs[Color_Attachment].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment_descs[Color_Attachment].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment_descs[Color_Attachment].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            // depth
            attachment_descs[DepthStencil_Attachment].format         = VK_FORMAT_D32_SFLOAT_S8_UINT;
            attachment_descs[DepthStencil_Attachment].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachment_descs[DepthStencil_Attachment].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment_descs[DepthStencil_Attachment].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachment_descs[DepthStencil_Attachment].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment_descs[DepthStencil_Attachment].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment_descs[DepthStencil_Attachment].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment_descs[DepthStencil_Attachment].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        VkAttachmentReference color_attach_ref {Color_Attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depth_attach_ref {DepthStencil_Attachment,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        std::vector<VkSubpassDescription> subpasses;
        subpasses.resize(1);
        subpasses[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[0].colorAttachmentCount    = 1;
        subpasses[0].pColorAttachments       = &color_attach_ref;
        subpasses[0].pDepthStencilAttachment = &depth_attach_ref;

        // create renderpass
        {
            VkRenderPassCreateInfo render_pass_info {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
            render_pass_info.attachmentCount = Attachment_Max_Count;
            render_pass_info.pAttachments    = attachment_descs;
            render_pass_info.subpassCount    = subpasses.size();
            render_pass_info.pSubpasses      = subpasses.data();
            render_pass_info.dependencyCount = 0;
            render_pass_info.pDependencies   = nullptr;

            VULKAN_API_CALL(vkCreateRenderPass(context->getDevice(), &render_pass_info, nullptr, &m_render_pass));
        }

        for (auto& framebuffer : m_framebuffers)
        {
            framebuffer.render_pass = m_render_pass;
        }

        auto vert_shader = std::make_unique<Shader>();
        auto vert_code   = readBinaryFile("G:\\Visual Studio Projects\\VkEngineDemo\\engine\\shaders\\vert.spv");
        vert_shader->Initialize(context, reinterpret_cast<const uint8_t*>(vert_code.data()), vert_code.size());

        auto frag_shader = std::make_unique<Shader>();
        auto frag_code   = readBinaryFile("G:\\Visual Studio Projects\\VkEngineDemo\\engine\\shaders\\frag.spv");
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

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {vert_shader_info, frag_shader_info};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        // VkVertexInputBindingDescription vertex_binding_desc = {};
        // vertex_binding_desc.binding                         = 0;
        // vertex_binding_desc.stride                          = sizeof(Vertex);
        // vertex_binding_desc.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;

        // std::array<VkVertexInputAttributeDescription, 4> vert_attr_desc = {};
        // vert_attr_desc[0].binding                                       = 0;
        // vert_attr_desc[0].location                                      = 0;
        // vert_attr_desc[0].format                                        = VK_FORMAT_R32G32B32_SFLOAT;
        // vert_attr_desc[0].offset                                        = offsetof(Vertex, px);

        // vert_attr_desc[1].binding  = 0;
        // vert_attr_desc[1].location = 1;
        // vert_attr_desc[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        // vert_attr_desc[1].offset   = offsetof(Vertex, nx);

        // vert_attr_desc[2].binding  = 0;
        // vert_attr_desc[2].location = 2;
        // vert_attr_desc[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
        // vert_attr_desc[2].offset   = offsetof(Vertex, tx);

        // vert_attr_desc[3].binding  = 0;
        // vert_attr_desc[3].location = 3;
        // vert_attr_desc[3].format   = VK_FORMAT_R32G32_SFLOAT;
        // vert_attr_desc[3].offset   = offsetof(Vertex, u);

        auto vertex_binding_desc = ShaderData::Vertex::getBindingDescription();
        auto vert_attr_desc      = ShaderData::Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount   = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vert_attr_desc.size());
        vertexInputInfo.pVertexBindingDescriptions      = &vertex_binding_desc;
        vertexInputInfo.pVertexAttributeDescriptions    = vert_attr_desc.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

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

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports    = &viewport;
        viewportState.scissorCount  = 1;
        viewportState.pScissors     = &scissor;

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
        multisampling.sampleShadingEnable  = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable     = VK_FALSE;
        color_blending.logicOp           = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount   = 1;
        color_blending.pAttachments      = &colorBlendAttachment;
        color_blending.blendConstants[0] = 0.0f;
        color_blending.blendConstants[1] = 0.0f;
        color_blending.blendConstants[2] = 0.0f;
        color_blending.blendConstants[3] = 0.0f;

        // std::vector<VkDescriptorSetLayout> layouts;
        // for (auto& layout : m_desc_array)
        //     layouts.push_back(layout->GetSetLayout());
        std::vector<VkDescriptorSetLayout> layouts{m_desc_tracker->GetSetLayout()};
        
        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount             = layouts.size();
        pipeline_layout_info.pSetLayouts                = layouts.data();

        VULKAN_API_CALL(
            vkCreatePipelineLayout(context->getDevice(), &pipeline_layout_info, nullptr, &m_pipeline_layout));

        std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
        dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
        dynamic_state_create_info.pDynamicStates    = dynamic_states.data();

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount                   = shaderStages.size();
        pipeline_info.pStages                      = shaderStages.data();
        pipeline_info.pVertexInputState            = &vertexInputInfo;
        pipeline_info.pInputAssemblyState          = &inputAssembly;
        pipeline_info.pViewportState               = &viewportState;
        pipeline_info.pRasterizationState          = &rasterizer;
        pipeline_info.pMultisampleState            = &multisampling;
        pipeline_info.pColorBlendState             = &color_blending;
        pipeline_info.pDynamicState                = &dynamic_state_create_info;
        pipeline_info.layout                       = m_pipeline_layout;
        pipeline_info.renderPass                   = m_render_pass;
        pipeline_info.subpass                      = 0;
        pipeline_info.basePipelineHandle           = VK_NULL_HANDLE;

        VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
        depth_stencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable       = VK_TRUE;
        depth_stencil.depthWriteEnable      = VK_TRUE;
        depth_stencil.depthCompareOp        = VK_COMPARE_OP_LESS;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;

        pipeline_info.pDepthStencilState = &depth_stencil;

        VULKAN_API_CALL(
            vkCreateGraphicsPipelines(context->getDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_pipeline));
    }

    void MainRenderPass::ResetPipeline()
    {
        auto& context = m_pass_info->render_context.vk_context;
        vkDestroyPipeline(context->getDevice(), m_pipeline, nullptr);
        vkDestroyPipelineLayout(context->getDevice(), m_pipeline_layout, nullptr);
    }

    void MainRenderPass::SetupSwapchainFramebuffer()
    {
        auto& context = m_pass_info->render_context.vk_context;

        size_t frames_in_flight = context->GetSwapchain().getImageCount();
        m_framebuffers.resize(frames_in_flight);
        auto swapchain_extent = context->GetSwapchain().getExtent();

        for (size_t i = 0; i < frames_in_flight; i++)
        {
            auto& color_attachment         = m_framebuffers[i].attachments[Color_Attachment];
            auto& depth_stencil_attachment = m_framebuffers[i].attachments[DepthStencil_Attachment];

            std::vector<VkImageView> attachments = {color_attachment->getView(), depth_stencil_attachment->getView()};

            VkFramebufferCreateInfo framebuffer_create_info = {};
            framebuffer_create_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass              = m_framebuffers[i].render_pass;
            framebuffer_create_info.attachmentCount         = attachments.size();
            framebuffer_create_info.pAttachments            = attachments.data();
            framebuffer_create_info.width                   = swapchain_extent.width;
            framebuffer_create_info.height                  = swapchain_extent.height;
            framebuffer_create_info.layers                  = 1;

            VULKAN_API_CALL(vkCreateFramebuffer(
                context->getDevice(), &framebuffer_create_info, nullptr, &m_framebuffers[i].framebuffer));
        }
    }

    void MainRenderPass::ResetSwapchainFramebuffer()
    {
        auto& context = m_pass_info->render_context.vk_context;

        for (auto swapchain_frambuffer : m_framebuffers)
        {
            vkDestroyFramebuffer(context->getDevice(), swapchain_frambuffer.framebuffer, nullptr);
            swapchain_frambuffer.framebuffer = VK_NULL_HANDLE;
        }
    }

    void MainRenderPass::PreDrawSetup()
    {
        // for (auto& framebuffer : m_framebuffers)
        // {
        //     framebuffer.attachments[Color_Attachment]->TransferLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        // }
    }

    void MainRenderPass::PostDrawCallback()
    {
        auto& context            = m_pass_info->render_context.vk_context;
        auto& frame_index        = context->GetFrameIndex();
        auto& active_framebuffer = m_framebuffers[frame_index];

        active_framebuffer.attachments[Color_Attachment]->TransferLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    void MainRenderPass::Draw()
    {
        auto&    context = m_pass_info->render_context.vk_context;
        uint32_t width = m_pass_info->width, height = m_pass_info->height;
        auto&    frame_index        = context->GetFrameIndex();
        auto&    active_framebuffer = m_framebuffers[frame_index];
        
        auto& command_buffers = context->GetCommandBuffers();
        command_buffers.ActivateFramebuffer(active_framebuffer);

        command_buffers.BeginRenderPass(active_framebuffer);
        command_buffers.BindPipeline(m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);

        m_desc_tracker->BindDescriptorSet(m_pipeline_layout, VK_PIPELINE_BIND_POINT_GRAPHICS);

        command_buffers.SetViewport(VkViewport {0.f, 0.f, (float)width, (float)height, 0.f, 1.f});
        command_buffers.SetScissor(VkRect2D {{0, 0}, {width, height}});

        // todo: is it necessary to bind everytime before drawing? if not relocate it later
        for (auto& mesh : m_meshes)
        {
            auto                      index_buffer       = mesh->GetBuffer(Index_Buffer);
            std::vector<VkBuffer>     bind_index_buffers = {index_buffer};
            std::vector<VkDeviceSize> bind_index_offsets = {0};
            command_buffers.Bind(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, bind_index_buffers, bind_index_offsets);
            
            auto                      vertex_buffer            = mesh->GetBuffer(Vertex_Buffer);
            auto                      vertex_count             = mesh->getVertexCount();
            std::vector<VkBuffer>     bind_vertex_buffers      = {vertex_buffer, vertex_buffer, vertex_buffer};
            std::vector<VkDeviceSize> bind_vertex_attr_offsets = {
                0, vertex_count * sizeof(glm::vec3), vertex_count * sizeof(glm::vec3) * 2};
            command_buffers.Bind(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bind_vertex_buffers, bind_vertex_attr_offsets);
            
            command_buffers.DrawIndexed(mesh.get());
        }

        command_buffers.EndRenderPass(active_framebuffer);
        command_buffers.Submit();

        PostDrawCallback();
    }

} // namespace Chandelier