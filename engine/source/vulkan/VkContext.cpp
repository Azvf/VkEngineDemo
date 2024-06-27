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
    void VulkanPhysicalDeviceFeatures::Query(VkPhysicalDevice phy_device, uint32_t api_version)
    {
        VkPhysicalDeviceFeatures2 phy_device_feat2 = {};
        phy_device_feat2.sType                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        phy_device_feat2.pNext = &Core_1_1;
        Core_1_1.sType         = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;

        if (api_version >= VK_API_VERSION_1_2)
        {
            Core_1_1.pNext = &Core_1_2;
            Core_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        }

        if (api_version >= VK_API_VERSION_1_3)
        {
            Core_1_2.pNext = &Core_1_3;
            Core_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        }

        vkGetPhysicalDeviceFeatures2(phy_device, &phy_device_feat2);

        Core_1_0 = phy_device_feat2;
        
        // // Apply config modifications
        // Core_1_0.robustBufferAccess = VK_TRUE;

        // // Apply platform restrictions
        // RestrictEnabledPhysicalDeviceFeatures(this);
        
        DeviceIdProps.sType       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES_KHR;
        DeviceSubgroupProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;

		DeviceProps.sType         = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
        DeviceProps.pNext = &DeviceIdProps;
        DeviceIdProps.pNext       = &DeviceSubgroupProps;

        vkGetPhysicalDeviceProperties2(phy_device, &DeviceProps);
    }

    VKContext::~VKContext() { UnInit(); }
    
    VulkanInstance::VulkanInstance() :
        instance_extensions({
            VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME,
        }),
        device_extensions({
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
        }),
        validation_layers({"VK_LAYER_KHRONOS_validation"}), 
        enable_validation_layer(true)
    {}

    VulkanInstance::~VulkanInstance() { UnInit(); }

    bool VulkanInstance::CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validation_layers)
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

    std::vector<const char*> VulkanInstance::GetRequiredExtensions()
    {
        std::vector<const char*> extensions;

        uint32_t     glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enable_validation_layer)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void VulkanInstance::Initialize()
    {
        if (enable_validation_layer && !CheckValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo {};
        appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName   = "Application Name: tiny engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName        = "Engine Name: tiny engine";
        appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion         = api_version;

        VkInstanceCreateInfo createInfo {};
        createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions                    = GetRequiredExtensions();
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        debugCreateInfo.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity                    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        
        // debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void*                                       pUserData) -> VkBool32 
        {
            if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                // Message is important enough to show
            }

            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

            return VK_FALSE;
        };

        if (enable_validation_layer)
        {
            createInfo.enabledLayerCount   = static_cast<uint32_t>(validation_layers.size());
            createInfo.ppEnabledLayerNames = validation_layers.data();
            createInfo.pNext               = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext             = nullptr;
        }

        VULKAN_API_CALL(vkCreateInstance(&createInfo, nullptr, &handle));

        if (enable_validation_layer)
        {
            VULKAN_API_CALL(createDebugUtilsMessengerEXT(handle, &debugCreateInfo, nullptr, &debug_messenger));
        }
    }

    void VulkanInstance::UnInit() {
        if (enable_validation_layer)
        {
            if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                    handle, "vkDestroyDebugUtilsMessengerEXT"))
            {
                func(handle, debug_messenger, nullptr);
            }
        }

        vkDestroyInstance(handle, nullptr);
    }

    std::optional<VkPhysicalDevice> DefaultGPUSelector::Fetch(VulkanInstance* vk_instance, VkSurfaceKHR surface)
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(vk_instance->handle, &device_count, nullptr);
        if (device_count == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(vk_instance->handle, &device_count, devices.data());

        for (const auto& device : devices)
        {
            QueueFamily queue_family = VKContext::FindQueueFamilies(device);

            bool extensionsSupported = VKContext::CheckDeviceExtensionSupport(device, vk_instance);

            if (queue_family.Complete() && extensionsSupported)
            {
                return device;
            }
        }

        return std::nullopt;
    }

    VulkanDevice::VulkanDevice(VulkanInstance* instance) 
        : vk_instance(instance)
    {}
    
    VulkanDevice::~VulkanDevice() { UnInit(); }

    void VulkanDevice::Initialize(std::unique_ptr<GPUSelector> gpu_selector, VkSurfaceKHR surface)
    { 
        assert(vk_instance && gpu_selector);

        auto vk_phy_device = gpu_selector->Fetch(vk_instance, surface);
        if (!vk_phy_device.has_value())
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
        
        phy_device = vk_phy_device.value();
        device_features.Query(phy_device, vk_instance->api_version);

        queue_family = VKContext::FindQueueFamilies(phy_device);
        
        this->SetupPresentQueue(surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t>                   uniqueQueueFamilies = {queue_family.gfx_queue_index.value(),
                                                                    queue_family.present_queue_index.value()};

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

        VkDeviceCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos    = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &device_features.Core_1_0.features;

        createInfo.enabledExtensionCount   = static_cast<uint32_t>(vk_instance->device_extensions.size());
        createInfo.ppEnabledExtensionNames = vk_instance->device_extensions.data();

        if (vk_instance->enable_validation_layer)
        {
            createInfo.enabledLayerCount   = static_cast<uint32_t>(vk_instance->validation_layers.size());
            createInfo.ppEnabledLayerNames = vk_instance->validation_layers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        VkPhysicalDeviceFeatures enabled_features  = {};
        enabled_features.drawIndirectFirstInstance = VK_TRUE;
        enabled_features.samplerAnisotropy         = device_features.Core_1_0.features.samplerAnisotropy;
        createInfo.pEnabledFeatures                = &enabled_features;

        // https://stackoverflow.com/questions/60592369/vulkan-timeline-semaphore-extension-cannot-be-enabled
        // timeline semaphore features needs to be set explictly like this
        VkPhysicalDeviceVulkan12Features vk12_features = {};
        vk12_features.sType                            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vk12_features.pNext                            = nullptr;
        vk12_features.timelineSemaphore                = true;

        createInfo.pNext = &vk12_features;

        VULKAN_API_CALL(vkCreateDevice(phy_device, &createInfo, nullptr, &device));

        vkGetDeviceQueue(device, queue_family.gfx_queue_index.value(), 0, &gfx_queue);
        vkGetDeviceQueue(device, queue_family.present_queue_index.value(), 0, &present_queue);
    }

    void VulkanDevice::UnInit() { }

    void VKContext::Initialize(std::shared_ptr<WindowSystem> window_system)
    {
        m_vk_instance = std::make_unique<VulkanInstance>();
        m_vk_instance->Initialize();

        m_surface = window_system->CreateSurface(m_vk_instance->handle);

        m_vk_device = std::make_unique<VulkanDevice>(m_vk_instance.get());
        m_vk_device->Initialize(std::make_unique<DefaultGPUSelector>(), m_surface);

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

        vkDestroySurfaceKHR(m_vk_instance->handle, m_surface, nullptr);
        vkDestroyDevice(getDevice(), nullptr);
    }

    VkInstance VKContext::getInstance() const { return m_vk_instance->handle; }

    VkDevice VKContext::getDevice() const
    {
        return m_vk_device->device;
    }

    VkPhysicalDevice VKContext::getPhysicalDevice() const { return m_vk_device->phy_device; }

    VkQueue VKContext::getGraphicsQueue() const { return m_vk_device->gfx_queue; }

    VkQueue VKContext::getPresentQueue() const { return m_vk_device->present_queue; }

    VkSurfaceKHR VKContext::getSurface() const { return m_surface; }

    const VulkanPhysicalDeviceFeatures& VKContext::GetDeviceFeatures() const { return m_vk_device->device_features; }

    DescriptorPools& VKContext::GetDescriptorPools() { return m_desc_pools; }

    SwapChain& VKContext::GetSwapchain() { return m_swapchain; }

    void VulkanDevice::SetupPresentQueue(VkSurfaceKHR surface) { 
        if (queue_family.present_queue_index.has_value())
        {
            return;
        }
        
        if (!queue_family.Complete())
        {
            return;
        }

        const auto support_present = [surface](VkPhysicalDevice phy_device, uint32_t queue_index) -> bool {
            VkBool32 supported = VK_FALSE;
            VULKAN_API_CALL(vkGetPhysicalDeviceSurfaceSupportKHR(phy_device, queue_index, surface, &supported));
            return (supported == VK_TRUE);
        };

        uint32_t gfx     = queue_family.gfx_queue_index.value();
        uint32_t compute = queue_family.compute_queue_index.value();
        // uint32_t transfer = queue_family.transfer_queue_index.value();

        bool gfx_support = std::invoke(support_present, phy_device, gfx);
        
        bool compute_support = std::invoke(support_present, phy_device, compute);

        if (gfx != compute && compute_support)
        {
            queue_family.present_queue_index.emplace(compute);
        }
        else
        {
            queue_family.present_queue_index.emplace(gfx);
        }
    }

    uint32_t VKContext::getGraphicsQueueFamilyIndex() const
    {
        return m_vk_device->queue_family.gfx_queue_index.value();
    }

    CommandBufferManager& VKContext::GetCommandManager() { 
        return m_command_manager;
    }

    VkFormat VKContext::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(getPhysicalDevice(), format, &props);

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


    void VKContext::CopyBufferToTexture(Buffer* buffer, Texture* texture, uint32_t layer, uint32_t mip_level)
    {
        VkDeviceSize buffer_size = buffer->GetBufferSize();

        uint32_t copy_width  = texture->getWidth() * std::pow(0.5, mip_level);
        uint32_t copy_height = texture->getHeight() * std::pow(0.5, mip_level);

        size_t       pixel_byte_size = TextureFormatToByteSize(texture->getFormat());
        VkDeviceSize tex_size        = copy_width * copy_height * pixel_byte_size;

        // if (buffer_size != tex_size)
        // {
        //     ENGINE_THROW_ERROR("buffer size and texture size not match", EngineCode::Buffer_Size_Not_Match);
        // }

        VkBufferImageCopy region = {};
        region.bufferOffset      = 0;
        region.bufferRowLength   = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = mip_level;
        region.imageSubresource.baseArrayLayer = layer;
        region.imageSubresource.layerCount     = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {copy_width, copy_height, 1};

        m_command_manager.Copy(buffer, texture, std::vector<VkBufferImageCopy> {region});
    }

    void VKContext::CopyTextureToBuffer(Texture* texture, Buffer* buffer, uint32_t layer, uint32_t mip_level)
    {
        VkDeviceSize buffer_size = buffer->GetBufferSize();
        
        uint32_t     copy_width  = texture->getWidth() * std::pow(0.5, mip_level);
        uint32_t     copy_height = texture->getHeight() * std::pow(0.5, mip_level);
        
        size_t       pixel_byte_size = TextureFormatToByteSize(texture->getFormat());
        VkDeviceSize tex_size        = copy_width * copy_height * pixel_byte_size;

        // if (buffer_size != tex_size)
        // {
        //     ENGINE_THROW_ERROR("buffer size and texture size not match", EngineCode::Buffer_Size_Not_Match);
        // }

        VkBufferImageCopy region = {};
        region.bufferOffset      = 0;
        region.bufferRowLength   = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = mip_level;
        region.imageSubresource.baseArrayLayer = layer;
        region.imageSubresource.layerCount     = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent   = {copy_width, copy_height, 1};

        m_command_manager.Copy(texture, buffer, std::vector<VkBufferImageCopy> {region});
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
            mappedRange.size   = buffer->GetMemorySize();

            mapped_ranges.push_back(mappedRange);
        }

        VULKAN_API_CALL(vkFlushMappedMemoryRanges(getDevice(), mapped_ranges.size(), mapped_ranges.data()));
    }

    QueueFamily VKContext::FindQueueFamilies(VkPhysicalDevice phy_device)
    {
        /*QueueFamilyIndices indices;

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_family_count, queue_families.data());

        for (int i = 0; i < queue_family_count; i++)
        {
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphics_family = i;
            }
            
            VkBool32 present_support = false;
            VULKAN_API_CALL(vkGetPhysicalDeviceSurfaceSupportKHR(phy_device, i, m_surface, &present_support));
            if (present_support)
            {
                indices.present_family = i;
            }

            if (indices.isComplete())
            {
                break;
            }

        }

        return indices;*/

        QueueFamily queue_family;
        
        uint32_t           queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_properties(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_family_count, queue_properties.data());

        for (int index = 0; index < queue_family_count; index++) {
            const auto& queue_prop = queue_properties[index];
            
            bool valid_queue = false;

            if (queue_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (!queue_family.gfx_queue_index.has_value())
                {
                    queue_family.gfx_queue_index.emplace(index);
                    valid_queue = true;
                }
                else
                {
                    // todo: Support for multi-queue/choose the best queue!
                }
            }
            else if (queue_prop.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                if (!queue_family.compute_queue_index.has_value() &&
                    (queue_family.gfx_queue_index.has_value() && queue_family.gfx_queue_index.value() != index))
                {
                    queue_family.compute_queue_index.emplace(index);
                    valid_queue = true;
                }
            }
            else if (queue_prop.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                if (!queue_family.transfer_queue_index.has_value() &&
                    !(queue_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
                    !(queue_prop.queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    queue_family.transfer_queue_index.emplace(index);
                    valid_queue = true;
                }
            }

            if (!valid_queue)
            {
                continue;
            }

        }

        return queue_family;
    }

    uint32_t VKContext::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(getPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        ENGINE_THROW_ERROR("failed to find suitable memory type!", EngineCode::None_Suitable_Mem_Type);
    }

    bool VKContext::CheckDeviceExtensionSupport(VkPhysicalDevice phy_device, VulkanInstance* vk_instance)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(phy_device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(phy_device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(vk_instance->device_extensions.begin(),
                                                 vk_instance->device_extensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
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
        assert(0 && "need to be revampped");
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
    
    VkSampleCountFlagBits VKContext::GetSuitableSampleCount() { 
        assert(VK_SAMPLE_COUNT_8_BIT <= GetMaxUsableSampledCount());
        if (VK_SAMPLE_COUNT_8_BIT <= GetMaxUsableSampledCount())
        {
            return VK_SAMPLE_COUNT_8_BIT;
        }
        return VK_SAMPLE_COUNT_1_BIT;
    }

    VkSampleCountFlagBits VKContext::GetMaxUsableSampledCount()
    {
        const auto& props = m_vk_device->device_features.DeviceProps.properties;
        
        VkSampleCountFlags counts = 
            props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts;

        if (counts & VK_SAMPLE_COUNT_64_BIT)
        {
            return VK_SAMPLE_COUNT_64_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_32_BIT)
        {
            return VK_SAMPLE_COUNT_32_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_16_BIT)
        {
            return VK_SAMPLE_COUNT_16_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_8_BIT)
        {
            return VK_SAMPLE_COUNT_8_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_4_BIT)
        {
            return VK_SAMPLE_COUNT_4_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_2_BIT)
        {
            return VK_SAMPLE_COUNT_2_BIT;
        }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    void VKContext::GenerateMipMaps(Texture* texture, int mipmap_levels)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(getPhysicalDevice(), texture->getFormat(), &props);
        if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            assert(0);
            return;
        }

        int32_t mip_width  = texture->getWidth();
        int32_t mip_height = texture->getHeight();
        int32_t layers     = texture->getLayers();

        VkImageMemoryBarrier barrier {};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image                           = texture->getImage();
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = layers;
        barrier.subresourceRange.levelCount     = 1; // 1 level a time

        for (uint32_t i = 1; i < mipmap_levels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

            m_command_manager.IssuePipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                   std::vector<VkImageMemoryBarrier> {barrier});

            VkImageBlit blit_info {};
            blit_info.srcOffsets[0]                 = {0, 0, 0};
            blit_info.srcOffsets[1]                 = {mip_width, mip_height, 1};
            blit_info.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit_info.srcSubresource.mipLevel       = i - 1;
            blit_info.srcSubresource.baseArrayLayer = 0;
            blit_info.srcSubresource.layerCount     = layers; // miplevel i-1 to i for all layers

            blit_info.dstOffsets[0] = {0, 0, 0};
            blit_info.dstOffsets[1] = {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1};
            blit_info.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit_info.dstSubresource.mipLevel       = i;
            blit_info.dstSubresource.baseArrayLayer = 0;
            blit_info.dstSubresource.layerCount     = layers;

            m_command_manager.Blit(texture->getImage(),
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   texture->getImage(),
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   std::vector<VkImageBlit> {blit_info});
        
            barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            m_command_manager.IssuePipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                   std::vector<VkImageMemoryBarrier> {barrier});
            if (mip_width > 1)
                mip_width /= 2;
            if (mip_height > 1)
                mip_height /= 2;     
        }

        // the last miplevel(miplevels - 1) change to shader_read
        barrier.subresourceRange.baseMipLevel = mipmap_levels - 1;
        barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

        m_command_manager.IssuePipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                               std::vector<VkImageMemoryBarrier> {barrier});
        
        texture->m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        m_command_manager.Submit();
    }


}

