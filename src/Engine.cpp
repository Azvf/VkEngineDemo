#include "Engine.h"

namespace vulkan {
	Engine::Engine()
		: m_window(WIDTH, HEIGHT, "Vk Engine Demo"),
		m_renderer((GLFWwindow*)m_window.getWindowHandle(), WIDTH, HEIGHT)
	{
		
	}

	void Engine::run()
	{
		while (!m_window.shouldClose()) {
			m_window.PollEvents();
			m_renderer.Render(WIDTH, HEIGHT);
		}
	}
}