#include "ui_pass.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include "runtime/core/base/exception.h"
#include "UI/window_system.h"
#include "main_pass.h"

#include "VkContext.h"
#include "CommandBuffers.h"
#include "Texture.h"

namespace Chandelier {
    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        printf("VkResult %d\n", err);
        if (err < 0)
            assert(0);
    }

    #define UI_PASS_SETUP_CONTEXT   auto& context         = m_pass_info->render_context.vk_context; \
                                    auto& window_system   = m_pass_info->window_system;             \
                                    auto& swapchain       = context->GetSwapchain();                \
                                    auto& command_manager = context->GetCommandManager();

	UIPass::~UIPass() { UnInit(); }

    void UIPass::Recreate() { 
        ResetImGuiPipelineContext();
        SetupImGuiPipelineContext();
    }

    void UIPass::SetupImGuiPipelineContext()
    {
        UI_PASS_SETUP_CONTEXT
        
        bool enable_msaa = m_pass_info->main_pass_uniform_buffer->config.anti_aliasing == Enable_MSAA;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)window_system->Handle(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance                  = context->getInstance();
        init_info.PhysicalDevice            = context->getPhysicalDevice();
        init_info.Device                    = context->getDevice();
        init_info.QueueFamily               = context->getGraphicsQueueFamilyIndex();
        init_info.Queue                     = context->getGraphicsQueue();
        init_info.PipelineCache             = VK_NULL_HANDLE;
        /**
         * @todo: what will happend if active descriptor pool changes?
         */
        init_info.DescriptorPool            = context->GetDescriptorPools().Handle();
        init_info.Allocator                 = nullptr;
        /**
         * @todo: may be different from the real swapchain image count,
         * see ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
         */
        init_info.MinImageCount             = static_cast<uint32_t>(swapchain.getImageCount());
        init_info.ImageCount                = static_cast<uint32_t>(swapchain.getImageCount());
        init_info.CheckVkResultFn           = check_vk_result;
        init_info.Subpass                   = UI_Pass;
        init_info.MSAASamples = (enable_msaa) ? context->GetSuitableSampleCount() : VK_SAMPLE_COUNT_1_BIT;
        /**
         * @info: not using m_render_pipeline in UI pass 
         * cause ImGui_ImplVulkan_Init has created them for us and we are drawing as main pass's subpass
         */
        assert(m_pass_info->render_pass);
        ImGui_ImplVulkan_Init(&init_info, *m_pass_info->render_pass);

        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void UIPass::ResetImGuiPipelineContext()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    //void UIPass::SetupRenderPass()
    //{
    //    UI_PASS_SETUP_CONTEXT

    //    VkAttachmentDescription attachment_description = {};
    //    attachment_description.format                  = VK_FORMAT_R16G16B16A16_SFLOAT;
    //    attachment_description.samples                 = VK_SAMPLE_COUNT_1_BIT;
    //    attachment_description.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //    attachment_description.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    //    attachment_description.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //    attachment_description.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //    attachment_description.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    //    attachment_description.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //    VkAttachmentReference attachment_ref {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    //    VkSubpassDescription subpass = {};
    //    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //    subpass.colorAttachmentCount = 1;
    //    subpass.pColorAttachments    = &attachment_ref;

    //    VkSubpassDependency dependencies[1] = {};
    //    dependencies[0].srcSubpass          = VK_SUBPASS_EXTERNAL;
    //    dependencies[0].dstSubpass          = 0;
    //    dependencies[0].srcStageMask        = VK_PIPELINE_STAGE_TRANSFER_BIT;
    //    dependencies[0].dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //    dependencies[0].srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
    //    dependencies[0].dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    //    VkRenderPassCreateInfo render_pass_info {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    //    render_pass_info.attachmentCount = 1;
    //    render_pass_info.pAttachments    = &attachment_description;
    //    render_pass_info.subpassCount    = 1;
    //    render_pass_info.pSubpasses      = &subpass;
    //    render_pass_info.dependencyCount = static_cast<uint32_t>(sizeof(dependencies) / sizeof(dependencies[0]));
    //    render_pass_info.pDependencies   = dependencies;

    //    VULKAN_API_CALL(
    //        vkCreateRenderPass(context->getDevice(), &render_pass_info, nullptr, m_pass_info->render_pass));

    //    for (auto& framebuffer : m_framebuffers)
    //    {
    //        framebuffer.render_pass = *m_pass_info->render_pass;
    //    }
    //}

    //void UIPass::ResetRenderPass()
    //{
    //    // auto& context = m_pass_info->render_context.vk_context;
    //    // vkDestroyPipeline(context->getDevice(), m_render_pipeline.pipeline, nullptr);
    //    // vkDestroyPipelineLayout(context->getDevice(), m_render_pipeline.layout, nullptr);
    //}

    /*void UIPass::SetupFramebuffers() {
        UI_PASS_SETUP_CONTEXT

        auto extent = swapchain.getExtent();
        m_framebuffers.resize(swapchain.getImageCount());
        for (size_t i = 0; i < swapchain.getImageCount(); ++i)
        {
            m_framebuffers[i].render_area = {{0, 0}, {extent.width, extent.height}};
            m_framebuffers[i].attachments.resize(1);
            
            std::vector<VkImageView> attachments = {m_framebuffers[i].attachments[Color_Attachment]->getView()};

            VkFramebufferCreateInfo framebuffer_create_info {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            framebuffer_create_info.renderPass      = *m_pass_info->render_pass;
            framebuffer_create_info.attachmentCount = attachments.size();
            framebuffer_create_info.pAttachments    = attachments.data();
            framebuffer_create_info.width           = extent.width;
            framebuffer_create_info.height          = extent.height;
            framebuffer_create_info.layers          = 1;

            VULKAN_API_CALL(vkCreateFramebuffer(
                context->getDevice(), &framebuffer_create_info, nullptr, &m_framebuffers[i].handle));
        }
    }

    void UIPass::ResetFramebuffers() {
        UI_PASS_SETUP_CONTEXT

        for (auto& framebuffer : m_framebuffers)
        {
            vkDestroyFramebuffer(context->getDevice(), framebuffer.handle, nullptr);
            framebuffer.handle = VK_NULL_HANDLE;
        }

        m_framebuffers.clear();
    }*/

    /*void UIPass::SetupAttachments()
    {
        UI_PASS_SETUP_CONTEXT

        uint32_t width = m_pass_info->width, height = m_pass_info->height;
        size_t   frames_in_flight = context->GetSwapchain().getImageCount();

        m_framebuffers.resize(frames_in_flight);

        for (int i = 0; i < frames_in_flight; i++)
        {
            m_framebuffers[i].render_area = {{0, 0}, {width, height}};
            m_framebuffers[i].attachments.resize(1);

            for (auto& attachment : m_framebuffers[i].attachments)
            {
                attachment = std::make_shared<Texture>();
            }

            auto& color_attachment = m_framebuffers[i].attachments[Color_Attachment];

            color_attachment->InitTex2D(context,
                                        width,
                                        height,
                                        1,
                                        1,
                                        VK_FORMAT_R16G16B16A16_SFLOAT,
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }
    }

    void UIPass::ResetAttachments()
    {
        for (auto& framebuffer : m_framebuffers)
        {
            for (auto& attachment : framebuffer.attachments)
            {
                attachment = nullptr;
            }
        }
    }*/

    void UIPass::UnInit()
    {
        // ResetFramebuffers();
        ResetImGuiPipelineContext();
        // ResetRenderPass();
        // ResetAttachments();
    }

	void UIPass::Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) {
		m_pass_info = std::dynamic_pointer_cast<UIPassInitInfo>(info);

        // SetupAttachments();
        // SetupRenderPass();
        SetupImGuiPipelineContext();
        // SetupFramebuffers();
    }

	void UIPass::PreDrawSetup() {

	}

    void UIPass::ImGuiSetCheckBox(const char* label, int32_t* v) { 
        bool checkbox_set = *v;
        ImGui::Checkbox(label, &checkbox_set);
        *v = checkbox_set;
    }
	
    void UIPass::ImGuiDraw() {
        auto& main_pass_ubo = m_pass_info->main_pass_uniform_buffer;

        #if 1
        struct FuncHolder
        {
            static bool ItemGetter(void* data, int idx, const char** out_str)
            {
                *out_str = ((std::string*)data)[idx].c_str();
                return true;
            }
        };

		ImGui::Begin("Chandelier Setting Menu");
        
		{
            int& display_texture_index = main_pass_ubo->config.display_texture;
            std::vector<std::string> texture_string_names {
                "None", "BaseColor", "Normal", "Height", "Metallic", "Roughness", "UV"};

            ImGui::Combo("Display Map",
                         &display_texture_index,
                         &FuncHolder::ItemGetter,
                         texture_string_names.data(),
                         texture_string_names.size());
		}

        {
            int& anti_aliasing = main_pass_ubo->config.anti_aliasing;
            std::vector<std::string> aa_string_names {"None", "MSAA x8"};
            ImGui::Combo("Anti_Aliasing",
                         &anti_aliasing,
                         &FuncHolder::ItemGetter,
                         aa_string_names.data(),
                         aa_string_names.size());
        }

        ImGuiSetCheckBox("show skybox", &main_pass_ubo->config.show_skybox);
        ImGuiSetCheckBox("rotating", &main_pass_ubo->config.rotating);
        ImGuiSetCheckBox("gamma correction", &main_pass_ubo->config.use_gamma_correction);
        ImGuiSetCheckBox("tone mapping", &main_pass_ubo->config.tone_mapping);
        
		ImGui::End();

        #else
        bool open_demo_window = true;
        ImGui::ShowDemoWindow(&open_demo_window);
        #endif
    }

	void UIPass::Draw()
    {
        UI_PASS_SETUP_CONTEXT
        
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiDraw();

        command_manager.RenderImGui();

        PostDrawCallback();
    }

	void UIPass::PostDrawCallback() {}

    const VkRenderPass* UIPass::GetRenderPass() { return m_pass_info->render_pass; }

}

