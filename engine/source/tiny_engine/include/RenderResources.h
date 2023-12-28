#pragma once

#include <vector>
#include <memory>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace vulkan
{
	enum
	{
		FRAMES_IN_FLIGHT = 2,
		SHADOW_RESOLUTION = 2048,
	};

	class Image;
	class Buffer;
	class SwapChain;

	struct RenderResources
	{
	public:
		explicit RenderResources(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool cmdPool, uint32_t width, uint32_t height, SwapChain* swapChain);
		
		RenderResources(const RenderResources&) = delete;
		RenderResources(const RenderResources&&) = delete;
		RenderResources& operator= (const RenderResources&) = delete;
		RenderResources& operator= (const RenderResources&&) = delete;

		~RenderResources();

		void resize(uint32_t width, uint32_t height);

	public:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VkCommandPool m_commandPool;
		SwapChain* m_swapChain;
		
		VkSemaphore m_swapChainImageAvailableSemaphores[FRAMES_IN_FLIGHT];
		VkSemaphore m_renderFinishedSemaphores[FRAMES_IN_FLIGHT];
		VkFence m_frameFinishedFence[FRAMES_IN_FLIGHT];
		
		VkCommandBuffer m_commandBuffers[FRAMES_IN_FLIGHT * 2];
		
		VkRenderPass m_mainRenderPass;

		VkFramebuffer m_mainFramebuffers[FRAMES_IN_FLIGHT];
		
		std::unique_ptr<Image> m_depthStencilImage[FRAMES_IN_FLIGHT];
		std::unique_ptr<Image> m_colorImage[FRAMES_IN_FLIGHT];
		std::unique_ptr<Image> m_diffuseImage[FRAMES_IN_FLIGHT];
		std::unique_ptr<Image> m_tonemappedImage[FRAMES_IN_FLIGHT];

		VkImageView m_depthImageView[FRAMES_IN_FLIGHT];

		std::pair<VkPipeline, VkPipelineLayout> m_lightingPipeline;
		
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSetLayout m_textureDescriptorSetLayout;
		VkDescriptorSet m_textureDescriptorSet;
		VkSampler textureSampler;

	// private:
	// 	void createResizableResources(uint32_t width, uint32_t height);
	// 	void destroyResizeableResources();
	};
}