#include "render_system.h"

#include <thread>
#include <chrono>

#include "UI/window_system.h"
#include "editor/camera.h"
#include "render/passes/main_pass.h"
#include "render/passes/skybox_pass.h"
#include "render/passes/ui_pass.h"
#include "render/precompute/brdf_lut.h"
#include "VkContext.h"

namespace Chandelier
{
    RenderSystem::~RenderSystem() { UnInit(); }

    void RenderSystem::Initialize(std::shared_ptr<WindowSystem> window_system)
    {
        m_window_system = window_system;
        m_context       = std::make_shared<VKContext>();
        m_context->Initialize(m_window_system);

        m_main_pass_uniform_buffer = std::make_shared<MainPassUniformBuffer>();
        m_main_pass_uniform_buffer->config.rotating      = true;
        // m_main_pass_uniform_buffer->config.show_skybox   = true;
        m_main_pass_uniform_buffer->config.anti_aliasing = Enable_MSAA;

        m_anti_aliasing = (eAntiAliasing)m_main_pass_uniform_buffer->config.anti_aliasing;

        auto main_pass_init_info                       = std::make_shared<MainRenderPassInitInfo>();
        main_pass_init_info->render_context.vk_context = m_context;
        main_pass_init_info->memory_uniform_buffer     = m_main_pass_uniform_buffer;
        main_pass_init_info->width                     = m_window_system->GetWindowSize().x;
        main_pass_init_info->height                    = m_window_system->GetWindowSize().y;

        m_main_pass = std::make_shared<MainRenderPass>();
        m_main_pass->Initialize(main_pass_init_info);

        auto skybox_pass_init_info                       = std::make_shared<SkyboxPassInitInfo>();
        skybox_pass_init_info->render_context.vk_context = m_context;
        skybox_pass_init_info->render_pass               = m_main_pass->GetRenderPass();
        skybox_pass_init_info->main_pass_uniform_buffer  = m_main_pass_uniform_buffer;
        skybox_pass_init_info->width                     = m_window_system->GetWindowSize().x;
        skybox_pass_init_info->height                    = m_window_system->GetWindowSize().y;

        m_skybox_pass = std::make_shared<SkyboxPass>();
        m_skybox_pass->Initialize(skybox_pass_init_info);

        auto ui_pass_init_info                       = std::make_shared<UIPassInitInfo>();
        ui_pass_init_info->render_context.vk_context = m_context;
        ui_pass_init_info->window_system             = m_window_system;
        ui_pass_init_info->main_pass_uniform_buffer  = m_main_pass_uniform_buffer;
        ui_pass_init_info->render_pass               = m_main_pass->GetRenderPass();
        ui_pass_init_info->width                     = m_window_system->GetWindowSize().x;
        ui_pass_init_info->height                    = m_window_system->GetWindowSize().y;
        
        m_ui_pass = std::make_shared<UIPass>();
        m_ui_pass->Initialize(ui_pass_init_info);

        auto brdflut_init_info            = std::make_shared<BRDFLutInitInfo>();
        brdflut_init_info->width          = 512;
        brdflut_init_info->height         = 512;
        brdflut_init_info->render_context.vk_context = m_context;

        m_lut_pass = std::make_shared<BRDFLutPass>();
        m_lut_pass->Initialize(brdflut_init_info);

        // m_camera       = std::make_shared<Camera>();
        // m_camera->type = Camera::CameraType::lookat;
        // // m_camera->setPosition(glm::vec3(0.f, -0.5f, -3.f));
        // m_camera->setRotation(glm::vec3(0.0f));
        // Vector2i fb_size = m_window_system->GetFramebufferSize();
        // m_camera->setPerspective(40.0f, fb_size.x / (float)fb_size.y, 0.01f, 256.0f);

        m_arcball_camera = std::make_shared<sss::ArcBallCamera>(glm::vec3(0.0f, 0.0f, 0.0f), 8.0f);
        // m_arcball_camera->update(glm::vec2(0.0, 45.0), 0.0);

        SetupLightingSet();

        PreRenderSetup();
    }

    void RenderSystem::UnInit() {}

    void RenderSystem::SetupLightingSet()
    {
        auto& lights = m_main_pass_uniform_buffer->lights;

        lights.point_light_num = 5;
        for (int i = 0; i < lights.point_light_num; i++)
        {
            lights.point_lights[i].color     = Vector3(1.0, 1.0, 1.0);
            lights.point_lights[i].intensity = 1000.0;
            lights.point_lights[i].radius    = 10.0;
        }

        float base_distance = 3.0;

        lights.point_lights[0].color    = Vector3(0.08, 0.29, 0.47);
        lights.point_lights[0].position = Vector3(base_distance, base_distance, base_distance);

        lights.point_lights[1].position = Vector3(-base_distance, -base_distance, -base_distance);
        lights.point_lights[2].position = Vector3(base_distance, -base_distance, base_distance);
        lights.point_lights[3].position = Vector3(-base_distance, -base_distance, base_distance);
    }

    void RenderSystem::PreRenderSetup() { m_main_pass->PreDrawSetup(); }

    void RenderSystem::Render() { 
        auto& swapchain = m_context->GetSwapchain();
        auto  extent    = swapchain.getExtent();
        float ar        = extent.width / float(extent.height);

        if (m_main_pass_uniform_buffer->config.rotating)
        {
            // m_camera->rotate(glm::vec3(0.f, 1.f, 0.f));
            m_arcball_camera->update(glm::vec2(1.0, 0.0), 0.0);
        }
        
        constexpr float fovy = glm::radians(40.0f);

        const glm::mat4 vulkanCorrection = {
            {1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f, 1.0f}};

        const glm::mat4 viewMatrix = m_arcball_camera->getViewMatrix();
        const glm::mat4 viewProjection =
            vulkanCorrection * glm::perspective(fovy, ar, 0.01f, 50.0f) * viewMatrix;

        SkyboxPassUniformBuffer skybox_ubo;
        skybox_ubo.inv_model_view_projection = glm::inverse(viewProjection);
        m_skybox_pass->UpdateUniformBuffer(skybox_ubo);

        m_main_pass_uniform_buffer->camera.position = glm::vec4(m_arcball_camera->getPosition(), 1.0);
        m_main_pass_uniform_buffer->camera.view       = viewMatrix;
        m_main_pass_uniform_buffer->camera.projection =
            vulkanCorrection * glm::perspective(fovy, ar, 0.01f, 50.0f);
        m_main_pass->UpdateUniformBuffer(*m_main_pass_uniform_buffer);

        m_need_recreate |= (m_anti_aliasing != m_main_pass_uniform_buffer->config.anti_aliasing);
        if (m_need_recreate)
        {
            m_main_pass->Recreate();
            m_skybox_pass->Recreate();
            m_ui_pass->Recreate();

            m_anti_aliasing = (eAntiAliasing)m_main_pass_uniform_buffer->config.anti_aliasing;
            m_need_recreate = false;
        }

        swapchain.AcquireImage(m_need_resize);

        if (m_need_resize)
        {
            m_main_pass->Recreate();
            m_need_resize = false;
        }

        std::vector<std::shared_ptr<RenderPass>> subpasses {m_skybox_pass, m_ui_pass};
        
        m_main_pass->ForwardDraw(subpasses);
        m_lut_pass->Draw();

        // m_context->TransferRenderPassResultToSwapchain(m_main_pass.get());

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        swapchain.SwapBuffer(m_need_resize);

        if (m_need_resize)
        {
            m_main_pass->Recreate();
            m_need_resize = false;
        }
    }

} // namespace Chandelier
