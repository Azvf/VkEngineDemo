#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <utility>

namespace vulkan
{
	namespace LightingPipeline
	{
		std::pair<VkPipeline, VkPipelineLayout> create(VkDevice device, VkRenderPass renderPass, uint32_t subpassIndex, uint32_t setLayoutCount, VkDescriptorSetLayout* descriptorSetLayout);
	}
}