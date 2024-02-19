#pragma once

#include <memory>

namespace Chandelier {
    class WindowSystem;
    class RenderSystem;

	class Engine {
	public:
		Engine();

		void Initialize();
        void UnInit();

	public:
		void Run();

	private:
        std::shared_ptr<WindowSystem> m_window_system;
        std::shared_ptr<RenderSystem> m_render_system;
	};
}