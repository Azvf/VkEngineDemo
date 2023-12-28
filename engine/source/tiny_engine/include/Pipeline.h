#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <utility>

namespace vulkan {

	class Pipeline {
	public:
		explicit Pipeline(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkExtent2D extent, VkDescriptorSetLayout descriptorSetLayout, VkRenderPass renderPass);
		~Pipeline();
	
	public:
		VkPipeline getPipeline() const;
		VkPipelineLayout getPipelineLayout() const;

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_graphicsPipeline;
	};

}