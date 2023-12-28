#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>

namespace vulkan
{
	class VKContext;

	class SwapChain
	{
	public:
		explicit SwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height);
		~SwapChain();
		
		SwapChain(SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		SwapChain& operator= (const SwapChain&) = delete;
		SwapChain& operator= (const SwapChain&&) = delete;
		
		VkExtent2D getExtent() const;
		VkFormat getImageFormat() const;
		operator VkSwapchainKHR() const;
		VkImage getImage(size_t index) const;
		VkImageView getImageView(size_t index) const;
		size_t getImageCount() const;
		VkRenderPass getRenderPass() const;
		VkFramebuffer getFramebuffer(uint32_t index) const;
		VkSwapchainKHR getSwapchain() const;

		void recreate(uint32_t width, uint32_t height);
	
	private:
		void createSwapChain(uint32_t width, uint32_t height);
		void createRenderPass();
		void createDepthBuffer();
		void createFramebuffers();
		void destroy();

	private:
		VkFormat findDepthFormat();
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VkSurfaceKHR m_surface;
		VkSwapchainKHR m_swapChain;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;

		VkRenderPass m_renderPass;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;

		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;
	};
}