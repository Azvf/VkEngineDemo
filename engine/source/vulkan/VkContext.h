#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>

namespace Chandelier {
	
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);

	class VKContext
	{
	public:
		explicit VKContext(void* windowHandle);
		~VKContext();
		
		VKContext(const VKContext&) = delete;
		VKContext(const VKContext&&) = delete;
		VKContext& operator= (const VKContext&) = delete;
		VKContext& operator= (const VKContext&&) = delete;
		
		VkInstance getInstance() const;
		VkDevice getDevice() const;
		VkPhysicalDevice getPhysicalDevice() const;
		VkQueue getGraphicsQueue() const;
		VkQueue getPresentQueue() const;
		VkCommandPool getGraphicsCommandPool() const;
		VkSurfaceKHR getSurface() const;
		VkDescriptorPool getDescriptorPool() const;
		// VkPhysicalDeviceFeatures getDeviceFeatures() const;
		// VkPhysicalDeviceFeatures getEnabledDeviceFeatures() const;
		// VkPhysicalDeviceProperties getDeviceProperties() const;
		uint32_t getGraphicsQueueFamilyIndex() const;
		VkQueryPool getQueryPool() const;

	public:
		void CreateSwapchain();
		void RecreateSwapChain();
		void ClearSwapChain();

	private:
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();

	private:
		void* m_windowHandle;
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VkSurfaceKHR m_surface;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		VkCommandPool m_graphicsCommandPool;
		VkDescriptorPool m_descriptorPool;
		// VkPhysicalDeviceFeatures m_features;
		// VkPhysicalDeviceFeatures m_enabledFeatures;
		// VkPhysicalDeviceProperties m_properties;
		uint32_t m_graphicsQueueFamilyIndex;
		VkQueryPool m_queryPool;
		VkSwapchainKHR m_swapchain;
		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_swapchainImageViews;
		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;
	};

}
