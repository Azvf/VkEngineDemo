#include "Engine.h"

#include "runtime/framework/global/global_context.h"
#include "UI/window_system.h"
#include "render/base/render_system.h"

namespace Chandelier
{
    Engine::Engine() {}

    void Engine::Initialize()
    {
        g_context.StartSystems("config path");

        m_window_system = std::make_shared<WindowSystem>(Vector2i {1920, 1080}, "tiny engine");
        m_window_system->Initialize();

        m_render_system = std::make_shared<RenderSystem>();
        m_render_system->Initialize(m_window_system);
    }

    void Engine::UnInit()
    {
        g_context.ShutdownSystems();
        m_render_system = nullptr;
        m_window_system = nullptr;
    }

    void Engine::Run()
    {
        while (!m_window_system->ShouldClose())
        {
            m_window_system->PollEvents();
            if (!m_window_system->IsIconified())
            {
                m_render_system->Render();
            }
        }
    }
} // namespace Chandelier