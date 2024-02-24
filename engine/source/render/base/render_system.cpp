#include "render_system.h"

#include <thread>
#include <chrono>

#include "UI/window_system.h"
#include "editor/camera.h"
#include "render/passes/main_pass.h"
#include "render/passes/skybox_pass.h"
#include "render/passes/ui_pass.h"
#include "VkContext.h"

namespace Chandelier
{
    RenderSystem::~RenderSystem() { UnInit(); }

    void RenderSystem::Initialize(std::shared_ptr<WindowSystem> window_system)
    {
        m_window_system = window_system;
        m_context       = std::make_shared<VKContext>();
        m_context->Initialize(m_window_system);

        auto main_pass_init_info                       = std::make_shared<MainRenderPassInitInfo>();
        main_pass_init_info->render_context.vk_context = m_context;
        main_pass_init_info->aa                        = AntiAliasing::None_AA;
        main_pass_init_info->width                     = m_window_system->GetWindowSize().x;
        main_pass_init_info->height                    = m_window_system->GetWindowSize().y;

        m_main_pass = std::make_shared<MainRenderPass>();
        m_main_pass->Initialize(main_pass_init_info);

        auto skybox_pass_init_info                       = std::make_shared<SkyboxPassInitInfo>();
        skybox_pass_init_info->render_context.vk_context = m_context;
        skybox_pass_init_info->render_pass               = m_main_pass->GetRenderPass();
        skybox_pass_init_info->width                     = m_window_system->GetWindowSize().x;
        skybox_pass_init_info->height                    = m_window_system->GetWindowSize().y;

        m_skybox_pass = std::make_shared<SkyboxPass>();
        m_skybox_pass->Initialize(skybox_pass_init_info);

        auto ui_pass_init_info                       = std::make_shared<UIPassInitInfo>();
        ui_pass_init_info->render_context.vk_context = m_context;
        ui_pass_init_info->window_system             = m_window_system;
        ui_pass_init_info->render_pass               = m_main_pass->GetRenderPass();
        ui_pass_init_info->width                     = m_window_system->GetWindowSize().x;
        ui_pass_init_info->height                    = m_window_system->GetWindowSize().y;
        
        m_ui_pass = std::make_shared<UIPass>();
        m_ui_pass->Initialize(ui_pass_init_info);

        // m_camera       = std::make_shared<Camera>();
        // m_camera->type = Camera::CameraType::lookat;
        // // m_camera->setPosition(glm::vec3(0.f, -0.5f, -3.f));
        // m_camera->setRotation(glm::vec3(0.0f));
        // Vector2i fb_size = m_window_system->GetFramebufferSize();
        // m_camera->setPerspective(40.0f, fb_size.x / (float)fb_size.y, 0.01f, 256.0f);

        m_arcball_camera = std::make_shared<sss::ArcBallCamera>(glm::vec3(0.0f, 0.25f, 0.0f), 1.0f);

    }

    void RenderSystem::UnInit() {}

    void RenderSystem::PreRenderSetup() { m_main_pass->PreDrawSetup(); }

    void RenderSystem::Render() { 
        auto& swapchain = m_context->GetSwapchain();

        // m_camera->rotate(glm::vec3(0.f, 1.f, 0.f));
        m_arcball_camera->update(glm::vec2(2.0, 0.0), 0.0);

        const float fovy = glm::radians(40.0f);

        // calculate view, projection and shadow matrix
        const glm::mat4 vulkanCorrection = {
            {1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f, 1.0f}};

        const glm::mat4 viewMatrix = m_arcball_camera->getViewMatrix();
        const glm::mat4 viewProjection =
            vulkanCorrection * glm::perspective(fovy, 1280 / float(720), 0.01f, 50.0f) * viewMatrix;

        SkyboxPassUniformBuffer skybox_ubo;
        // skybox_ubo.camera_position = m_camera->position;
        // skybox_ubo.view_projection_mat = m_camera->matrices.perspective * m_camera->matrices.view;
        // skybox_ubo.inv_model_view_projection = glm::inverse(m_camera->matrices.perspective * m_camera->matrices.view);
        skybox_ubo.inv_model_view_projection = glm::inverse(viewProjection);
        
        m_skybox_pass->UpdateUniformBuffer(skybox_ubo);

        MainPassUniformBuffer main_ubo;
        // main_ubo.model_view = m_camera->matrices.view * glm::mat4(1.0f);
        // main_ubo.projection = m_camera->matrices.perspective;
        // main_ubo.projection[1][1] *= -1;
        main_ubo.model_view = viewMatrix * glm::mat4(1.0f);
        main_ubo.projection = vulkanCorrection * glm::perspective(fovy, 1280 / float(720), 0.01f, 50.0f);
        m_main_pass->UpdateUniformBuffer(main_ubo);

        swapchain.AcquireImage(m_resized);

        if (m_resized)
        {
            m_main_pass->Resize();
            m_resized = false;
        }

        std::vector<std::shared_ptr<RenderPass>> subpasses {m_skybox_pass, m_ui_pass};
        
        m_main_pass->ForwardDraw(subpasses);
        
        m_context->TransferRenderPassResultToSwapchain(m_main_pass.get());

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        swapchain.SwapBuffer(m_resized);

        if (m_resized)
        {
            m_main_pass->Resize();
            m_resized = false;
        }
    }

} // namespace Chandelier
