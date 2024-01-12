#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Chandelier {
	class Sampler {
	public:
		explicit Sampler(VkPhysicalDevice physicalDevice, VkDevice device);
		~Sampler();

	public:
		void createTextureSampler();
	
	public:
		VkSampler getTextureSampler() const;

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VkSampler m_textureSampler;
	};

}
