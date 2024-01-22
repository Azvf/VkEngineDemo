#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Chandelier {
	class CommandBuffer {
	public:
		CommandBuffer() = default;
		~CommandBuffer();

		friend class VKContext;

	private:
		VkCommandBuffer command_buffer;
	};

}