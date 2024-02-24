#pragma once

#include "VkCommon.h"
#include "ArcBallCamera.h"

class Camera;
namespace Chandelier
{
    class WindowSystem;
    class RenderScene;

    class MainRenderPass;
    class SkyboxPass;
    class UIPass;

    class RenderSystem
    {
    public:
        RenderSystem() = default;
        ~RenderSystem();

    public:
        void Initialize(std::shared_ptr<WindowSystem> window_system);
        void UnInit();

        void PreRenderSetup();
        void Render();

    private:
        std::shared_ptr<VKContext>      m_context;
        std::shared_ptr<WindowSystem>   m_window_system;
        // std::shared_ptr<Camera>         m_camera;
        std::shared_ptr<sss::ArcBallCamera>         m_arcball_camera;
        // std::shared_ptr<RenderScene>    m_scene;
        std::shared_ptr<MainRenderPass> m_main_pass;
        std::shared_ptr<SkyboxPass>     m_skybox_pass;
        std::shared_ptr<UIPass>         m_ui_pass;

        bool m_resized = false;
    };
} // namespace Chandelier