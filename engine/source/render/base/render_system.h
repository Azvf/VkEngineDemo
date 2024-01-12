#pragma once

#include <memory>
#include "render_resources.h"

namespace Chandelier {
	class WindowSystem;
	class VKContext;

	class RenderSystem {
	public:
		RenderSystem() = default;
		~RenderSystem();

	public:
		void Initialize(std::shared_ptr<WindowSystem> window_system);

	private:
		std::shared_ptr<VKContext> m_context;
		RenderResource m_resource;
	};
}