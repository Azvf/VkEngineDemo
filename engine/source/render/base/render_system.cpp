#include "render_system.h"

#include "UI/window_system.h"
#include "editor/camera.h"
#include "render/passes/main_pass.h"
#include "render/passes/ui_pass.h"

namespace Chandelier
{
    RenderSystem::~RenderSystem() {}

    void RenderSystem::Initialize(std::shared_ptr<WindowSystem> window_system)
    {
        m_window_system = window_system;
        m_context       = std::make_shared<VKContext>(window_system);

        auto main_pass_init_info                       = std::make_shared<MainRenderPassInitInfo>();
        main_pass_init_info->render_context.vk_context = m_context;
        main_pass_init_info->aa                        = AntiAliasing::None_AA;

        m_main_pass = std::make_shared<MainRenderPass>();
        m_main_pass->Initialize(main_pass_init_info);

        auto ui_pass_init_info                       = std::make_shared<UIPassInitInfo>();
        ui_pass_init_info->render_context.vk_context = m_context;

        m_ui_pass = std::make_shared<UIPass>();
        m_ui_pass->Initialize(ui_pass_init_info);

        // Setup a default look-at camera
        m_camera->type = Camera::CameraType::lookat;
        m_camera->setPosition(glm::vec3(0.0f, -0.25f, -.5f));
        m_camera->setRotation(glm::vec3(0.0f));
        Vector2i fb_size = m_window_system->GetFramebufferSize();
        m_camera->setPerspective(40.0f, fb_size.x / (float)fb_size.y, 0.01f, 256.0f);
    }

} // namespace Chandelier
