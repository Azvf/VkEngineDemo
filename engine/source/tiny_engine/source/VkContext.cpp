#include "vkContext.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <set>
#include <iostream>

#include "RenderCfg.h"
#include "VKUtil.h"

namespace vulkan {

	VKContext::VKContext(void* windowHandle)
	{
        // create VkInstance
        {
            if (enableValidationLayers && !checkValidationLayerSupport()) {
                throw std::runtime_error("validation layers requested, but not available!");
            }

            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            auto extensions = getRequiredExtensions();
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();

                populateDebugMessengerCreateInfo(debugCreateInfo);
                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else {
                createInfo.enabledLayerCount = 0;

                createInfo.pNext = nullptr;
            }

            if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
                throw std::runtime_error("failed to create instance!");
            }
        }
        
        // create debug messenger
        {
            // if (!enableValidationLayers) return;
            if (enableValidationLayers) {
                VkDebugUtilsMessengerCreateInfoEXT createInfo;
                populateDebugMessengerCreateInfo(createInfo);

                _ASSERT(createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugUtilsMessenger) == VK_SUCCESS);
            }
        }

        // create surface 
        {
            if (glfwCreateWindowSurface(m_instance, (GLFWwindow*)windowHandle, nullptr, &m_surface) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }
        }

        // pick physical device
        {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
            if (deviceCount == 0) {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }
            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

            for (const auto& device : devices) {
                if (isDeviceSuitable(device, m_surface)) {
                    m_physicalDevice = device;
                    break;
                }
            }

            if (m_physicalDevice == VK_NULL_HANDLE) {
                throw std::runtime_error("failed to find a suitable GPU!");
            }

        }

        // create logical device
        {
            QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);

            uint32_t adapter_index = static_cast<uint32_t>(indices.graphicsFamily.value());
            if (adapter_index >= 0) {
                m_graphicsQueueFamilyIndex = adapter_index;
            }

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkPhysicalDeviceFeatures deviceFeatures{};

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
            else {
                createInfo.enabledLayerCount = 0;
            }

            if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
                throw std::runtime_error("failed to create logical device!");
            }

            vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
            vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
        }

        // create command pool
        {
            QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice, m_surface);

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;

            if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_graphicsCommandPool) != VK_SUCCESS) {
                throw std::runtime_error("failed to create command pool!");
            }
        }

        // create descriptor pool
        {
            std::array<VkDescriptorPoolSize, 2> poolSizes{};
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

            if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }
        }

	}

    VKContext::~VKContext() {
        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
        vkDestroyCommandPool(m_device, m_graphicsCommandPool, nullptr);
        vkDestroyDevice(m_device, nullptr);

        if (enableValidationLayers)
        {
            if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"))
            {
                func(m_instance, m_debugUtilsMessenger, nullptr);
            }
        }

        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    VkInstance VKContext::getInstance() const
    {
        return m_instance;
    }

    VkDevice VKContext::getDevice() const
    {
        return m_device;
    }

    VkPhysicalDevice VKContext::getPhysicalDevice() const
    {
        return m_physicalDevice;
    }

    VkQueue VKContext::getGraphicsQueue() const
    {
        return m_graphicsQueue;
    }

    VkQueue VKContext::getPresentQueue() const
    {
        return m_presentQueue;
    }

    VkCommandPool VKContext::getGraphicsCommandPool() const
    {
        return m_graphicsCommandPool;
    }

    VkSurfaceKHR VKContext::getSurface() const
    {
        return m_surface;
    }

    VkDescriptorPool VKContext::getDescriptorPool() const
    {
        return m_descriptorPool;
    }

    uint32_t VKContext::getGraphicsQueueFamilyIndex() const {
        return m_graphicsQueueFamilyIndex;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            // Message is important enough to show
        }

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

}

