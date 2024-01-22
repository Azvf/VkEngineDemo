#pragma once

#include "vulkan/VkCreateInfo.h"
#include <optional>

namespace Chandelier
{
    class WindowSystem;
    class Texture;
    class Buffer;
    class Shader;
    class CommandBuffer;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

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
        VkCommandPool    getGraphicsCommandPool() const;
        VkSurfaceKHR     getSurface() const;
        VkDescriptorPool getDescriptorPool() const;
        // VkPhysicalDeviceFeatures getDeviceFeatures() const;
        // VkPhysicalDeviceFeatures getEnabledDeviceFeatures() const;
        // VkPhysicalDeviceProperties getDeviceProperties() const;
        uint32_t    getGraphicsQueueFamilyIndex() const;
        VkQueryPool getQueryPool() const;

    public:
        void TransiteTextureLayout(std::shared_ptr<Texture> texture, VkImageLayout new_layout);
        void CopyBufferToTexture(std::shared_ptr<Buffer> buffer, std::shared_ptr<Texture> texture);

        std::shared_ptr<CommandBuffer> BeginSingleTimeCommand();
        void                           EndSingleTimeCommand(std::shared_ptr<CommandBuffer> command);

        QueueFamilyIndices FindQueueFamilies();
        uint32_t           FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    private:
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags         features);
        VkFormat FindDepthFormat();

    private:
        std::shared_ptr<WindowSystem> m_window_system;
        VkInstance                    m_instance;
        VkDebugUtilsMessengerEXT      m_debugUtilsMessenger;
        VkPhysicalDevice              m_physicalDevice;
        VkDevice                      m_device;
        VkSurfaceKHR                  m_surface;
        VkQueue                       m_graphicsQueue;
        VkQueue                       m_presentQueue;
        VkCommandPool                 m_graphicsCommandPool;
        VkDescriptorPool              m_descriptorPool;
        uint32_t                      m_graphicsQueueFamilyIndex;
        VkQueryPool                   m_queryPool;
        VkSwapchainKHR                m_swapchain;
        std::vector<VkImage>          m_swapchainImages;
        std::vector<VkImageView>      m_swapchainImageViews;
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
