#pragma once

#include <vector>
#include <memory>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "VkCreateInfo.h"

namespace Chandelier {
	class Uniform {
	public:
		// Uniform(VkPhysicalDevice physicalDevice, VkDevice device);
		Uniform() = default;
		~Uniform();

		void createUniformBuffers();

		void Initialize(const UniformCreateInfo& info);
		static std::shared_ptr<Uniform> Create(const UniformCreateInfo& info);

		VkBuffer getUniformBuffer(uint32_t index) const;
		VkDeviceMemory getUniformMemory(uint32_t index) const;

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;

		std::vector<VkBuffer> m_uniformBuffers;
		std::vector<VkDeviceMemory> m_uniformBuffersMemory;

		UniformCreateInfo m_info;
	};

}