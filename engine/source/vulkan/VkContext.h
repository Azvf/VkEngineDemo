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
    class VulkanInstance;
    class VulkanDevice;

    class WindowSystem;
    class Texture;
    class Buffer;
    class Shader;

    struct QueueFamily
    {
        std::optional<uint32_t> gfx_queue_index;
        std::optional<uint32_t> compute_queue_index;
        std::optional<uint32_t> transfer_queue_index;
        
        std::optional<uint32_t> present_queue_index;

        bool Complete()
        {
            return 
                gfx_queue_index.has_value() && 
                compute_queue_index.has_value() && 
                transfer_queue_index.has_value();
        }
    };

    struct VulkanPhysicalDeviceFeatures
    {
        friend class VKContext;

    public:
        VulkanPhysicalDeviceFeatures()  = default;
        ~VulkanPhysicalDeviceFeatures() = default;

        void Query(VkPhysicalDevice phy_device, uint32_t api_version);


        // Extension specific properties
        VkPhysicalDeviceIDPropertiesKHR    DeviceIdProps       = {};
        VkPhysicalDeviceSubgroupProperties DeviceSubgroupProps = {};

        VkPhysicalDeviceProperties2KHR     DeviceProps         = {};

        VkPhysicalDeviceFeatures2        Core_1_0 = {};
        VkPhysicalDeviceVulkan11Features Core_1_1 = {};

    private:
        // Anything above Core 1.1 cannot be assumed, they should only be used by the context at init time
        VkPhysicalDeviceVulkan12Features Core_1_2 = {};
        VkPhysicalDeviceVulkan13Features Core_1_3 = {};
    };

    using VulkanInstancePtr = std::unique_ptr<VulkanInstance>;
    class VulkanInstance
    {
        friend class VKContext;
        friend class VulkanDevice;
        friend class DefaultGPUSelector;

    public:
        VulkanInstance();
        ~VulkanInstance();

        void Initialize();
        void UnInit();
    
    private:
        bool CheckValidationLayerSupport();
        std::vector<const char*> GetRequiredExtensions();

    private:
        VkInstance handle = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
        
        const uint32_t api_version = VK_API_VERSION_1_2;

        const std::vector<const char*> instance_extensions;
        const std::vector<const char*> device_extensions;
        const std::vector<const char*> validation_layers;

        const bool enable_validation_layer = true;
    };

    class GPUSelector
    {
    public:
        virtual std::optional<VkPhysicalDevice> Fetch(VulkanInstance* vk_instance, VkSurfaceKHR surface) = 0;
    };

    class DefaultGPUSelector : public GPUSelector
    {
    public:
        DefaultGPUSelector() = default;
        std::optional<VkPhysicalDevice> Fetch(VulkanInstance* vk_instance, VkSurfaceKHR surface) override;
    };

    using VulkanDevicePtr = std::unique_ptr<VulkanDevice>;
    class VulkanDevice
    {
        friend class VKContext;

    public:
        VulkanDevice(VulkanInstance* instance);
        ~VulkanDevice();

        void Initialize(std::unique_ptr<GPUSelector> gpu_selector, VkSurfaceKHR surface);
        void UnInit();

    private:
        void SetupPresentQueue(VkSurfaceKHR surface);

    private:
        VulkanInstance*  vk_instance  = nullptr;
        
        VkPhysicalDevice phy_device   = VK_NULL_HANDLE;
        VkDevice         device       = VK_NULL_HANDLE;
        
        QueueFamily                  queue_family;
        VulkanPhysicalDeviceFeatures device_features;

        VkQueue gfx_queue     = VK_NULL_HANDLE;
        VkQueue present_queue = VK_NULL_HANDLE;
    };

    using VKContextPtr = std::shared_ptr<VKContext>;
    class VKContext : public std::enable_shared_from_this<VKContext>
    {
        friend class VulkanInstance;
        friend class VulkanDevice;
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
        uint32_t                  getGraphicsQueueFamilyIndex() const;
        
        const VulkanPhysicalDeviceFeatures& GetDeviceFeatures() const;

        CommandBufferManager& GetCommandManager();
        DescriptorPools&      GetDescriptorPools();
        SwapChain&            GetSwapchain();

        static QueueFamily FindQueueFamilies(VkPhysicalDevice phy_device);
        static bool        CheckDeviceExtensionSupport(VkPhysicalDevice phy_device, VulkanInstance* vk_instance);

    public:
        void TransiteTextureLayout(Texture* texture, VkImageLayout new_layout);
        void CopyBufferToTexture(Buffer* buffer, Texture* texture, uint32_t layer = 0, uint32_t mip_level = 0);
        void CopyTextureToBuffer(Texture* texture, Buffer* buffer, uint32_t layer = 0, uint32_t mip_level = 0);

        void FlushMappedBuffers(std::vector<Buffer*> mapped_buffers);

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        void                        IncFrameIndex();
        const std::atomic_uint64_t& GetFrameIndex();

        Sampler& GetSampler(const GPUSamplerState& sampler_state);

        void TransferRenderPassResultToSwapchain(const RenderPass* render_pass);

        VkSampleCountFlagBits GetSuitableSampleCount();

        void GenerateMipMaps(Texture* texture, int mipmap_levels);

    private:
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags         features);

        VkFormat FindDepthFormat();

        VkSampleCountFlagBits GetMaxUsableSampledCount();

    private:
        VulkanInstancePtr m_vk_instance;
        VulkanDevicePtr   m_vk_device;
        
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        
        CommandBufferManager  m_command_manager;
        SwapChain             m_swapchain;
        DescriptorPools       m_desc_pools;
        SamplerManager        m_sampler_manager;

        std::atomic_uint64_t m_frame_index = {};
    };

} // namespace Chandelier
