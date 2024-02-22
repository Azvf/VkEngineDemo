#pragma once

#include "render/base/render_pass.h"

#include "CommandBuffers.h"
#include "DescriptorPool.h"
#include "Sampler.h"
#include "SwapChain.h"
#include "VkCommon.h"

namespace Chandelier
{
    class VKContext;

    class WindowSystem;
    class Texture;
    class Buffer;
    class Shader;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

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
        VKContext() = default;
        virtual ~VKContext();

        void Initialize(std::shared_ptr<WindowSystem> window_system);
        void UnInit();

        VkInstance                getInstance() const;
        VkDevice                  getDevice() const;
        VkPhysicalDevice          getPhysicalDevice() const;
        VkQueue                   getGraphicsQueue() const;
        VkQueue                   getPresentQueue() const;
        VkSurfaceKHR              getSurface() const;
        VkPhysicalDeviceFeatures2 getDeviceFeatures() const;
        uint32_t                  getGraphicsQueueFamilyIndex() const;
        VkQueryPool               getQueryPool() const;

        CommandBufferManager& GetCommandManager();
        DescriptorPools&      GetDescriptorPools();
        SwapChain&            GetSwapchain();

    public:
        void TransiteTextureLayout(Texture* texture, VkImageLayout new_layout);
        void CopyBufferToTexture(Buffer* buffer, Texture* texture);
        void CopyTextureToBuffer(Texture* texture, Buffer* buffer);

        void FlushMappedBuffers(std::vector<Buffer*> mapped_buffers);

        QueueFamilyIndices      FindQueueFamilies(VkPhysicalDevice phy_device);
        uint32_t                FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        bool                    DeviceSuitable(VkPhysicalDevice phy_device);
        bool                    CheckDeviceExtensionSupport(VkPhysicalDevice phy_device);
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice phy_device);

        void                        IncFrameIndex();
        const std::atomic_uint64_t& GetFrameIndex();

        Sampler& GetSampler(const GPUSamplerState& sampler_state);

        void TransferRenderPassResultToSwapchain(const RenderPass* render_pass);

    private:
        bool CheckValidationLayerSupport();

        std::vector<const char*> GetRequiredExtensions();
        
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags         features);

        VkFormat FindDepthFormat();

        std::vector<VkBufferImageCopy> BuildCopyRegions(Buffer* buffer, Texture* texture);

    private:
        VkInstance                 m_instance;
        VkDebugUtilsMessengerEXT   m_debugUtilsMessenger;
        VkPhysicalDevice           m_physicalDevice;
        VkDevice                   m_device;
        VkSurfaceKHR               m_surface;
        VkQueue                    m_graphicsQueue;
        VkQueue                    m_presentQueue;
        uint32_t                   m_graphicsQueueFamilyIndex;
        VkPhysicalDeviceProperties m_properties;

        /** Features support. */
        VkPhysicalDeviceFeatures2        m_features      = {};
        VkPhysicalDeviceVulkan11Features m_vk11_features = {};
        VkPhysicalDeviceVulkan12Features m_vk12_features = {};

        VkQueryPool m_queryPool;

        CommandBufferManager  m_command_manager;
        SwapChain             m_swapchain;
        DescriptorPools       m_desc_pools;
        SamplerManager        m_sampler_manager;

        std::atomic_uint64_t m_frame_index = {};
    };

} // namespace Chandelier
