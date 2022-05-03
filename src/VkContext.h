#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>

namespace vulkan {
	
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
		VkCommandPool getGraphicsCommandPool() const;
		VkSurfaceKHR getSurface() const;
		// VkPhysicalDeviceFeatures getDeviceFeatures() const;
		// VkPhysicalDeviceFeatures getEnabledDeviceFeatures() const;
		// VkPhysicalDeviceProperties getDeviceProperties() const;
		// uint32_t getGraphicsQueueFamilyIndex() const;

	private:
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VkSurfaceKHR m_surface;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		VkCommandPool m_graphicsCommandPool;
		// VkPhysicalDeviceFeatures m_features;
		// VkPhysicalDeviceFeatures m_enabledFeatures;
		// VkPhysicalDeviceProperties m_properties;
		// uint32_t m_graphicsQueueFamilyIndex;
	};

}
