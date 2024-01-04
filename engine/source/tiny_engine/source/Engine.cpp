#include "Engine.h"

namespace Chandelier {
	Engine::Engine()
		: m_window(WIDTH, HEIGHT, "Vk Engine Demo"),
			m_renderer((GLFWwindow*)m_window.getWindowHandle(), WIDTH, HEIGHT)
	{
	}

	void Engine::run()
	{
		while (!m_window.shouldClose()) {
			if (!m_window.isIconified()) {
				m_window.PollEvents();
				m_renderer.Render(WIDTH, HEIGHT, m_window.getUserInput());
			}
		}
	}
}