#pragma once

#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "Window.h"
#include "Renderer.h"

namespace Chandelier {
	const uint32_t WIDTH = 1280;
	const uint32_t HEIGHT = 720;

	class Engine {
	public:
		Engine();

	public:
		void run();

	private:
		Window m_window;
		Renderer m_renderer;
	};
}