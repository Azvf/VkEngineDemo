#pragma once

#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace vulkan {
	class Uniform {
	public:
		Uniform(VkPhysicalDevice physicalDevice, VkDevice device);

	public:
		void createUniformBuffers();

	public:
		VkBuffer getUniformBuffer(uint32_t index) const;
		VkDeviceMemory getUniformMemory(uint32_t index) const;

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;

		std::vector<VkBuffer> m_uniformBuffers;
		std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	};

}