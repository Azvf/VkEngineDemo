#include "vkContext.h"

#include <set>
#include <iostream>
#include <thread>

#include "runtime/core/base/exception.h"
#include "UI/window_system.h"

#include "RenderCfg.h"
#include "SwapChain.h"
#include "Image.h"
#include "Texture.h"
#include "Buffer.h"
#include "Shader.h"
#include "CommandBuffer.h"
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

    VKContext::VKContext(std::shared_ptr<WindowSystem> window_system) : m_window_system(window_system)
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
        m_surface = window_system->CreateSurface(shared_from_this());

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
        // {
        //     QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice, m_surface);
        // 
        //     VkCommandPoolCreateInfo poolInfo{};
        //     poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        //     poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        //     poolInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
        // 
        //     if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_graphicsCommandPool) != VK_SUCCESS) {
        //         throw std::runtime_error("failed to create command pool!");
        //     }
        // }
        m_command_buffers = std::make_shared<CommandBuffers>();
        m_command_buffers->Initialize(shared_from_this());

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
        // ClearSwapChain();
        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
        //vkDestroyCommandPool(m_device, m_graphicsCommandPool, nullptr);
        m_command_buffers = nullptr;
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

    VkQueryPool VKContext::getQueryPool() const {
        return m_queryPool;
    }

    std::shared_ptr<CommandBuffers> VKContext::GetCommandBuffers() { 
        return m_command_buffers;
    }

    //void VKContext::CreateSwapchain() {
    //    // create SwapChain
    //    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice, m_surface);

    //    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    //    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

    //    Vector2i size = m_window_system->GetFramebufferSize();
    //    
    //    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, size.x, size.y);

    //    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    //    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    //        imageCount = swapChainSupport.capabilities.maxImageCount;
    //    }

    //    VkSwapchainCreateInfoKHR createInfo{};
    //    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    //    createInfo.surface = m_surface;
    //    createInfo.minImageCount = imageCount;
    //    createInfo.imageFormat = surfaceFormat.format;
    //    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    //    createInfo.imageExtent = extent;
    //    createInfo.imageArrayLayers = 1;
    //    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);
    //    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    //    if (indices.graphicsFamily != indices.presentFamily) {
    //        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    //        createInfo.queueFamilyIndexCount = 2;
    //        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    //    }
    //    else {
    //        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //    }

    //    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    //    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    //    createInfo.presentMode = presentMode;
    //    createInfo.clipped = VK_TRUE;
    //    createInfo.oldSwapchain = VK_NULL_HANDLE;

    //    VULKAN_API_CALL(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain)); 

    //    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    //    m_swapchainImages.resize(imageCount);
    //    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

    //    // create image view
    //    m_swapchainImageViews.resize(m_swapchainImages.size());
    //    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
    //        VkImageViewCreateInfo createInfo{};
    //        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //        createInfo.image = m_swapchainImages[i];
    //        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    //        createInfo.format = surfaceFormat.format;

    //        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    //        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    //        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    //        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    //        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //        createInfo.subresourceRange.baseMipLevel = 0;
    //        createInfo.subresourceRange.levelCount = 1;
    //        createInfo.subresourceRange.baseArrayLayer = 0;
    //        createInfo.subresourceRange.layerCount = 1;

    //        VULKAN_API_CALL(vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchainImageViews[i]));
    //    }

    //    // VkImageCreateInfo image_create_info{};
    //    // image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    //    // image_create_info.flags = 0;
    //    // image_create_info.imageType = VK_IMAGE_TYPE_2D;
    //    // image_create_info.extent.width = extent.width;
    //    // image_create_info.extent.height = extent.height;
    //    // image_create_info.extent.depth = 1;
    //    // image_create_info.mipLevels = 1;
    //    // image_create_info.arrayLayers = 1;
    //    // image_create_info.format = FindDepthFormat();
    //    // image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    //    // image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //    // image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    //    // image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    //    // image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //    // 
    //    // // @todo: optimize createImage interface to simplify image info init
    //    // createImage(
    //    //     m_physicalDevice, 
    //    //     m_device, 
    //    //     image_create_info,
    //    //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    //    //     m_depthImage,
    //    //     m_depthImageMemory
    //    // );
    //}

    //void VKContext::RecreateSwapChain() {
    //    //int width = 0, height = 0;
    //    Vector2i size = m_window_system->GetFramebufferSize();
    //    while (size.x == 0 || size.y == 0)
    //    {
    //        // minimized 0,0, pause for now
    //        size = m_window_system->GetFramebufferSize();
    //        m_window_system->WaitEvents();
    //        //std::this_thread::sleep_for(1ms);
    //    }

    //    vkDeviceWaitIdle(m_device);
    //    ClearSwapChain();

    //    CreateSwapchain();
    //}

    //void VKContext::ClearSwapChain() {
    //    // vkDestroyImageView(m_device, m_depthImageView, nullptr);
    //    // vkDestroyImage(m_device, m_depthImage, nullptr);
    //    // vkFreeMemory(m_device, m_depthImageMemory, nullptr);

    //    for (auto imageView : m_swapchainImageViews)
    //    {
    //        vkDestroyImageView(m_device, imageView, NULL);
    //    }
    //    vkDestroySwapchainKHR(m_device, m_swapchain, NULL); // also swapchain images
    //}

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

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat VKContext::FindDepthFormat() {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void VKContext::TransiteTextureLayout(std::shared_ptr<Texture> texture, VkImageLayout new_layout) {
        auto command = BeginSingleTimeCommand();
        
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
            ENGINE_THROW_ERROR("image layout transfer not supported", EngineCode::Image_Layout_Not_Supported);
        }
        
        vkCmdPipelineBarrier(
            command->command_buffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        
        EndSingleTimeCommand(command);
    }

    void VKContext::CopyBufferToTexture(std::shared_ptr<Buffer> buffer, std::shared_ptr<Texture> texture) {
        VkDeviceSize buffer_size = buffer->m_size;
        VkDeviceSize tex_size = texture->getWidth() * texture->getHeight() * 4;
        if (buffer_size != tex_size) {
            ENGINE_THROW_ERROR("buffer size and texture size not match", EngineCode::Buffer_Size_Not_Match);
        }
        
        auto command = BeginSingleTimeCommand();
        
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {texture->getWidth(), texture->getHeight(), 1};
        
        vkCmdCopyBufferToImage(
            command->command_buffer,
            buffer->m_buffer,
            texture->getImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
        
        EndSingleTimeCommand(command);
    }

    std::shared_ptr<CommandBuffer> VKContext::BeginSingleTimeCommand() {
        auto command = std::make_shared<CommandBuffer>();
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_command_buffers;
        allocInfo.commandBufferCount = 1;
        VULKAN_API_CALL(vkAllocateCommandBuffers(m_device, &allocInfo, &command->command_buffer));
        
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VULKAN_API_CALL(vkBeginCommandBuffer(command->command_buffer, &beginInfo));
        
        return command;
    }

    void VKContext::EndSingleTimeCommand(std::shared_ptr<CommandBuffer> command) {
        VULKAN_API_CALL(vkEndCommandBuffer(command->command_buffer));
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command->command_buffer;
        
        VULKAN_API_CALL(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
        VULKAN_API_CALL(vkQueueWaitIdle(m_graphicsQueue));
        vkFreeCommandBuffers(m_device, m_graphicsCommandPool, 1, &command->command_buffer);
    }


    QueueFamilyIndices VKContext::FindQueueFamilies()
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);

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

}

