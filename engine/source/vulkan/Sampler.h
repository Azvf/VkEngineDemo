#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "VkCreateInfo.h"

namespace Chandelier {
	class Sampler {
	public:
		Sampler() = default;
		~Sampler();

		void Initialize(const SamplerCreateInfo& info);
		static std::shared_ptr<Sampler> Create(const SamplerCreateInfo& info);

		void createTextureSampler();
	
		VkSampler getTextureSampler() const;

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VkSampler m_textureSampler;
	
		SamplerCreateInfo m_info;
	};

}
