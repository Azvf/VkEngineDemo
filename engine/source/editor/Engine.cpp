#include "Engine.h"

#include "runtime/framework/global/global_context.h"
#include "UI/window_system.h"
#include "editor/render_system.h"

namespace Chandelier
{
    Engine::Engine() {}

    void Engine::Initialize(const EngineInitInfo& info)
    {
        GlobalContextInitInfo g_context_init_info = {};
        g_context_init_info.executable_path = info.executable_path;
        
        g_context.Initialize(g_context_init_info);

        m_window_system = std::make_shared<WindowSystem>(Vector2i {960, 540}, "tiny engine");
        m_window_system->Initialize();

        m_render_system = std::make_shared<RenderSystem>();
        m_render_system->Initialize(m_window_system);
    }

    void Engine::UnInit()
    {
        m_render_system = nullptr;
        m_window_system = nullptr;

        g_context.UnInit();
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