#include "vkContext.h"

#include <set>
#include <iostream>
#include <thread>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "runtime/core/base/exception.h"
#include "UI/window_system.h"

#include "SwapChain.h"
#include "Texture.h"
#include "Buffer.h"
#include "Shader.h"
#include "CommandBuffers.h"
#include "Descriptor.h"
#include "Uniform.h"
#include "Sampler.h"

#include "VKUtil.h"

namespace Chandelier {
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void*                                       pUserData)
    {

        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            // Message is important enough to show
        }

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    
    const std::vector<const char*> instanceExtensions = {
        VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME,
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
    };

    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    VKContext::~VKContext() { UnInit(); }

    void VKContext::Initialize(std::shared_ptr<WindowSystem> window_system)
    {
        // create VkInstance
        {
            if (enableValidationLayers && !CheckValidationLayerSupport())
            {
                throw std::runtime_error("validation layers requested, but not available!");
            }

            VkApplicationInfo appInfo {};
            appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName   = "Application Name: tiny engine";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName        = "Engine Name: tiny engine";
            appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion         = VK_API_VERSION_1_2;

            VkInstanceCreateInfo createInfo {};
            createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            auto extensions                    = GetRequiredExtensions();
            createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
            if (enableValidationLayers)
            {
                createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();

                populateDebugMessengerCreateInfo(debugCreateInfo);
                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else
            {
                createInfo.enabledLayerCount = 0;

                createInfo.pNext = nullptr;
            }

            VULKAN_API_CALL(vkCreateInstance(&createInfo, nullptr, &m_instance));
        }

        // create debug messenger
        {
            if (enableValidationLayers)
            {
                VkDebugUtilsMessengerCreateInfoEXT createInfo;
                populateDebugMessengerCreateInfo(createInfo);

                VULKAN_API_CALL(createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugUtilsMessenger));
            }
        }

        // create surface
        m_surface = window_system->CreateSurface(shared_from_this());

        // pick physical device
        {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
            if (deviceCount == 0)
            {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }
            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

            for (const auto& device : devices)
            {
                if (DeviceSuitable(device))
                {
                    m_physicalDevice = device;
                    // vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);

                    // more comprehend features
                    VkPhysicalDeviceFeatures2 features2 = {};
                    features2.sType                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                    m_vk11_features.sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                    m_vk12_features.sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

                    m_vk11_features.pNext = &m_vk12_features;
                    features2.pNext       = &m_vk11_features;

                    vkGetPhysicalDeviceFeatures2(m_physicalDevice, &features2);
                    m_features = features2;

                    vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
                    break;
                }
            }

            if (m_physicalDevice == VK_NULL_HANDLE)
            {
                throw std::runtime_error("failed to find a suitable GPU!");
            }
        }

        // create logical device
        {
            QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

            uint32_t adapter_index = static_cast<uint32_t>(indices.graphicsFamily.value());
            if (adapter_index >= 0)
            {
                m_graphicsQueueFamilyIndex = adapter_index;
            }

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies)
            {
                VkDeviceQueueCreateInfo queueCreateInfo {};
                queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount       = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkPhysicalDeviceFeatures deviceFeatures {};

            VkDeviceCreateInfo createInfo {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos    = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (enableValidationLayers)
            {
                createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
            else
            {
                createInfo.enabledLayerCount = 0;
            }

            VkPhysicalDeviceFeatures enabled_features  = {};
            enabled_features.drawIndirectFirstInstance = VK_TRUE;
            enabled_features.samplerAnisotropy         = m_features.features.samplerAnisotropy;
            createInfo.pEnabledFeatures                = &enabled_features;

            // https://stackoverflow.com/questions/60592369/vulkan-timeline-semaphore-extension-cannot-be-enabled
            // timeline semaphore features needs to be set explictly like this
            VkPhysicalDeviceVulkan12Features vk12_features = {};
            vk12_features.sType                            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            vk12_features.pNext                            = nullptr;
            vk12_features.timelineSemaphore                = true;

            createInfo.pNext = &vk12_features;

            VULKAN_API_CALL(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));

            vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
            vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
        }

        m_command_manager.Initialize(shared_from_this());

        m_desc_pools.Initialize(shared_from_this());

        m_swapchain.Initialize(shared_from_this(), window_system);

        m_sampler_manager.Initialize(shared_from_this());
    }

    void VKContext::UnInit()
    {
        m_swapchain.UnInit();
        m_command_manager.UnInit();
        m_desc_pools.Free();
        vkDestroyDevice(m_device, nullptr);

        if (enableValidationLayers)
        {
            if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                    m_instance, "vkDestroyDebugUtilsMessengerEXT"))
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

    VkSurfaceKHR VKContext::getSurface() const
    {
        return m_surface;
    }

    VkPhysicalDeviceFeatures2 VKContext::getDeviceFeatures() const { return m_features; }

    DescriptorPools& VKContext::GetDescriptorPools()
    {
        return m_desc_pools;
    }

    SwapChain& VKContext::GetSwapchain() { return m_swapchain; }

    uint32_t VKContext::getGraphicsQueueFamilyIndex() const {
        return m_graphicsQueueFamilyIndex;
    }

    VkQueryPool VKContext::getQueryPool() const {
        return m_queryPool;
    }

    CommandBufferManager& VKContext::GetCommandManager() { 
        return m_command_manager;
    }

    VkFormat VKContext::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        ENGINE_THROW_ERROR("failed to find supported format!", EngineCode::General_Assert_Code);
    }

    VkFormat VKContext::FindDepthFormat() {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void VKContext::TransiteTextureLayout(Texture* texture, VkImageLayout new_layout) {
        VkImageMemoryBarrier barrier{};
        VkImageLayout old_layout = texture->getLayout();
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        
        barrier.image = texture->getImage();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;
        
        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else {
            ENGINE_THROW_ERROR("image layout transfer not supported", EngineCode::General_Assert_Code);
        }
        
        m_command_manager.IssuePipelineBarrier(
            sourceStage, destinationStage, std::vector<VkImageMemoryBarrier> {barrier});
    }

    void VKContext::CopyBufferToTexture(Buffer* buffer, Texture* texture) {
        VkDeviceSize buffer_size = buffer->getSize();
        VkDeviceSize tex_size = texture->getWidth() * texture->getHeight() * 4;
        if (buffer_size != tex_size) {
            ENGINE_THROW_ERROR("buffer size and texture size not match", EngineCode::Buffer_Size_Not_Match);
        }
        
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {texture->getWidth(), texture->getHeight(), 1};
        
        m_command_manager.Copy(buffer, texture, std::vector<VkBufferImageCopy> {region});
    }

    void VKContext::FlushMappedBuffers(std::vector<Buffer*> mapped_buffers) {
        std::vector<VkMappedMemoryRange> mapped_ranges;
        
        VkDeviceSize offset = {};
        for (const auto& buffer : mapped_buffers)
        {
            VkMappedMemoryRange mappedRange {};
            mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedRange.memory = buffer->getMemory();
            mappedRange.offset = buffer->getOffset();
            mappedRange.size   = buffer->getSize();

            mapped_ranges.push_back(mappedRange);
        }

        VULKAN_API_CALL(vkFlushMappedMemoryRanges(m_device, mapped_ranges.size(), mapped_ranges.data()));
    }

    QueueFamilyIndices VKContext::FindQueueFamilies(VkPhysicalDevice phy_device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            VULKAN_API_CALL(vkGetPhysicalDeviceSurfaceSupportKHR(phy_device, i, m_surface, &presentSupport));

            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    uint32_t VKContext::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        ENGINE_THROW_ERROR("failed to find suitable memory type!", EngineCode::None_Suitable_Mem_Type);
    }

    bool VKContext::DeviceSuitable(VkPhysicalDevice phy_device)
    {
        QueueFamilyIndices indices = FindQueueFamilies(phy_device);

        bool extensionsSupported = CheckDeviceExtensionSupport(phy_device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(phy_device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

     bool VKContext::CheckDeviceExtensionSupport(VkPhysicalDevice phy_device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(phy_device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(phy_device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

     SwapChainSupportDetails VKContext::QuerySwapChainSupport(VkPhysicalDevice phy_device)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy_device, m_surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, m_surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, m_surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(phy_device, m_surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                phy_device, m_surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    void VKContext::IncFrameIndex()
    {
        int frames_in_flight = m_swapchain.getImageCount();
        m_frame_index        = (m_frame_index + 1) % frames_in_flight;
    }
     
    const std::atomic_uint64_t& VKContext::GetFrameIndex() { return m_frame_index; }

     Sampler& VKContext::GetSampler(const GPUSamplerState& sampler_state)
     {
         return m_sampler_manager.GetSampler(sampler_state);
     }

    void VKContext::TransferRenderPassResultToSwapchain(const RenderPass* render_pass) {
        auto render_pass_attachment = render_pass->m_framebuffers[m_frame_index].attachments[Color_Attachment];
        auto extent = m_swapchain.getExtent();
        std::vector<VkImageBlit> regions;

        VkImageBlit blit_region {};
        blit_region.srcOffsets[0]                 = {0, 0, 0};
        blit_region.srcOffsets[1]                 = {(int)extent.width, (int)extent.height, 1};
        blit_region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blit_region.srcSubresource.mipLevel       = 0;
        blit_region.srcSubresource.baseArrayLayer = 0;
        blit_region.srcSubresource.layerCount     = 1;

        blit_region.dstOffsets[0]                 = {0, 0, 0};
        blit_region.dstOffsets[1]                 = {(int)extent.width, (int)extent.height, 1};
        blit_region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blit_region.dstSubresource.mipLevel       = 0;
        blit_region.dstSubresource.baseArrayLayer = 0;
        blit_region.dstSubresource.layerCount     = 1;

        regions.push_back(blit_region);

        // auto render_pass_attachment_layout = render_pass_attachment->getLayout();
        auto render_pass_attachment_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        render_pass_attachment->TransferLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        m_swapchain.TransferSwapchainImage(m_frame_index, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        m_command_manager.Blit(render_pass_attachment->getImage(),
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               m_swapchain.getImage(m_frame_index),
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               regions);
        
        render_pass_attachment->TransferLayout(render_pass_attachment_layout);
        m_swapchain.TransferSwapchainImage(m_frame_index, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        
        m_command_manager.Submit();
    }

    std::vector<const char*> VKContext::GetRequiredExtensions()
    {
        std::vector<const char*> extensions;

        uint32_t     glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);
        
        // extensions.insert(extensions.end(), instanceExtensions.begin(), instanceExtensions.end());

        if (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool VKContext::CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }
}

