#pragma once

#include <memory>
#include <string>

namespace Chandelier {
    class WindowSystem;
    class RenderSystem;

	struct EngineInitInfo
	{
        std::string executable_path;
	};

	class Engine {
	public:
		Engine();

		void Initialize(const EngineInitInfo& info);
        void UnInit();

	public:
		void Run();

	private:
        std::shared_ptr<WindowSystem> m_window_system;
        std::shared_ptr<RenderSystem> m_render_system;
	};
}