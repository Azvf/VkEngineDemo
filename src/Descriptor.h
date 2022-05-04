#pragma once

#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace vulkan {
	class Uniform;
	class Sampler;

	class Descriptor {
	public:
		Descriptor(VkDevice device, Uniform uni, VkImageView imageView, Sampler sampler);
		~Descriptor();

	public:
		const VkDescriptorSet& getDescriptorSet(uint32_t index) const;
		VkDescriptorSetLayout getDescriptorLayout() const;
		VkDescriptorPool getDescriptorPool() const;

	private:
		void createDescriptorPool();
		void createDescriptorSetLayout();
		void createDescriptorSets(Uniform uni, VkImageView imageView, Sampler sampler);
	
	private:
		VkDevice m_device;

		VkDescriptorPool m_descriptorPool;
		VkDescriptorSetLayout m_descriptorSetLayout;
		std::vector<VkDescriptorSet> m_descriptorSets;
	
	};
}