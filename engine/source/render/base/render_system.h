#pragma once

#include "render/base/render_pass.h"
#include "ArcBallCamera.h"

class Camera;
namespace Chandelier
{
    class WindowSystem;
    class RenderScene;

    class MainRenderPass;
    class SkyboxPass;
    class UIPass;
    class BRDFLutPass;

    class MainPassUniformBuffer;

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
        void SetupLightingSet();

    private:
        std::shared_ptr<VKContext>      m_context;
        std::shared_ptr<WindowSystem>   m_window_system;
        // std::shared_ptr<Camera>         m_camera;
        std::shared_ptr<sss::ArcBallCamera>         m_arcball_camera;
        // std::shared_ptr<RenderScene>    m_scene;
        std::shared_ptr<MainRenderPass> m_main_pass;
        std::shared_ptr<SkyboxPass>     m_skybox_pass;
        std::shared_ptr<UIPass>         m_ui_pass;
        std::shared_ptr<BRDFLutPass>    m_lut_pass;

        std::shared_ptr<MainPassUniformBuffer> m_main_pass_uniform_buffer;

        bool m_need_resize   = false;
        bool m_need_recreate = false;
        
        eAntiAliasing m_anti_aliasing;
    };
} // namespace Chandelier