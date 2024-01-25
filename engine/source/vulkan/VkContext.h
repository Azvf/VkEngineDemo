#pragma once

#include <optional>

#include "CommandBuffers.h"
#include "DescriptorPool.h"
#include "SwapChain.h"
#include "vulkan/VkCreateInfo.h"

namespace Chandelier
{
    class VKContext;

    class WindowSystem;
    class Texture;
    class Buffer;
    class Shader;
    class CommandBuffers;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    using VKContextPtr = std::shared_ptr<VKContext>;
    class VKContext : public std::enable_shared_from_this<VKContext>
    {
    public:
        explicit VKContext(std::shared_ptr<WindowSystem> window_system);
        ~VKContext();

        VkInstance       getInstance() const;
        VkDevice         getDevice() const;
        VkPhysicalDevice getPhysicalDevice() const;
        VkQueue          getGraphicsQueue() const;
        VkQueue          getPresentQueue() const;
        VkSurfaceKHR     getSurface() const;
        // VkPhysicalDeviceFeatures getDeviceFeatures() const;
        // VkPhysicalDeviceFeatures getEnabledDeviceFeatures() const;
        // VkPhysicalDeviceProperties getDeviceProperties() const;
        uint32_t    getGraphicsQueueFamilyIndex() const;
        VkQueryPool getQueryPool() const;

        CommandBuffers& GetCommandBuffers();
        DescriptorPools& GetDescriptorPools();

    public:
        void TransiteTextureLayout(std::shared_ptr<Texture> texture, VkImageLayout new_layout);
        void CopyBufferToTexture(std::shared_ptr<Buffer> buffer, std::shared_ptr<Texture> texture);

        QueueFamilyIndices FindQueueFamilies();
        uint32_t           FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        bool               DeviceSuitable();
        bool               CheckDeviceExtensionSupport();
        SwapChainSupportDetails QuerySwapChainSupport();

    private:
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags         features);
        VkFormat FindDepthFormat();

    private:
        std::shared_ptr<WindowSystem> m_window_system;

        VkInstance               m_instance;
        VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
        VkPhysicalDevice         m_physicalDevice;
        VkDevice                 m_device;
        VkSurfaceKHR             m_surface;
        VkQueue                  m_graphicsQueue;
        VkQueue                  m_presentQueue;
        uint32_t                 m_graphicsQueueFamilyIndex;

        // VkDescriptorPool m_descriptorPool;
        VkQueryPool      m_queryPool;

        CommandBuffers  m_command_buffers;
        SwapChain       m_swapchain;
        DescriptorPools m_desc_pools;

        // VkSwapchainKHR                  m_swapchain;
        // std::vector<VkImage>            m_swapchainImages;
        // std::vector<VkImageView>        m_swapchainImageViews;
        // VkPhysicalDeviceFeatures m_features;
        // VkPhysicalDeviceFeatures m_enabledFeatures;
        // VkPhysicalDeviceProperties m_properties;
        //
        // VkImage						m_depthImage;
        // VkImage						m_depthImage;
        // VkDeviceMemory				m_depthImageMemory;
        // VkImageView					m_depthImageView;
    };

} // namespace Chandelier
