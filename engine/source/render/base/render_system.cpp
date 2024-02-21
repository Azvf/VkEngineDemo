#include "render_system.h"

#include <thread>
#include <chrono>

#include "UI/window_system.h"
#include "editor/camera.h"
#include "render/passes/main_pass.h"
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

        auto ui_pass_init_info                       = std::make_shared<UIPassInitInfo>();
        ui_pass_init_info->render_context.vk_context = m_context;
        ui_pass_init_info->window_system             = m_window_system;
        ui_pass_init_info->render_pass               = m_main_pass->GetRenderPass();
        ui_pass_init_info->width                     = m_window_system->GetWindowSize().x;
        ui_pass_init_info->height                    = m_window_system->GetWindowSize().y;
        
        m_ui_pass = std::make_shared<UIPass>();
        m_ui_pass->Initialize(ui_pass_init_info);

        m_camera       = std::make_shared<Camera>();
        m_camera->type = Camera::CameraType::lookat;
        m_camera->setPosition(glm::vec3(0.f, -0.5f, -3.f));
        m_camera->setRotation(glm::vec3(0.0f));
        Vector2i fb_size = m_window_system->GetFramebufferSize();
        m_camera->setPerspective(40.0f, fb_size.x / (float)fb_size.y, 0.01f, 256.0f);
    }

    void RenderSystem::UnInit() {}

    void RenderSystem::PreRenderSetup() { m_main_pass->PreDrawSetup(); }

    void RenderSystem::Render() { 
        auto& swapchain = m_context->GetSwapchain();

        m_camera->rotate(glm::vec3(0.f, 1.f, 0.f));

        MainPassUniformBuffer host_ubo;
        host_ubo.model_view = m_camera->matrices.view * glm::mat4(1.0f);
        host_ubo.projection = m_camera->matrices.perspective;
        host_ubo.projection[1][1] *= -1;
        m_main_pass->UpdateUniformBuffer(host_ubo);

        swapchain.AcquireImage(m_resized);

        if (m_resized)
        {
            m_main_pass->Resize();
            m_resized = false;
        }

        std::vector<std::shared_ptr<RenderPass>> subpasses {m_ui_pass};
        
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
