#pragma once

#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace vulkan {
	class Uniform;
	class Sampler;

	class Descriptor {
	public:
		Descriptor(VkDevice device, VkImageView imageView, const Uniform& uni, const Sampler& sampler);
		~Descriptor();

	public:
		const VkDescriptorSet& getDescriptorSet(uint32_t index) const;
		VkDescriptorSetLayout getDescriptorLayout() const;
		VkDescriptorPool getDescriptorPool() const;

	private:
		void createDescriptorPool();
		void createDescriptorSetLayout();
		void createDescriptorSets(VkImageView imageView, const Uniform& uni, const Sampler& sampler);
	
	private:
		VkDevice m_device;

		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorPool m_descriptorPool;
		std::vector<VkDescriptorSet> m_descriptorSets;
	
	};
}