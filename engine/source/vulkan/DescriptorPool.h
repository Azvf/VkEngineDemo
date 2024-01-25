#pragma once

#include <memory>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "VkCreateInfo.h"

namespace Chandelier
{
    class Descriptor;

    class DescriptorPools
    {
        /**
         * Pool sizes to use. When one descriptor pool is requested to allocate a descriptor but
         * isn't able to do so, it will fail.
         *
         * Better defaults should be set later on, when we know more about our resource usage.
         */
        static constexpr uint32_t POOL_SIZE_STORAGE_BUFFER         = 1024;
        static constexpr uint32_t POOL_SIZE_DESCRIPTOR_SETS        = 1024;
        static constexpr uint32_t POOL_SIZE_STORAGE_IMAGE          = 1024;
        static constexpr uint32_t POOL_SIZE_COMBINED_IMAGE_SAMPLER = 1024;
        static constexpr uint32_t POOL_SIZE_UNIFORM_BUFFER         = 1024;
        static constexpr uint32_t POOL_SIZE_UNIFORM_TEXEL_BUFFER   = 1024;

        std::shared_ptr<VKContext>    m_context;
        std::vector<VkDescriptorPool> m_pools;

        int64_t m_active_pool_index = 0;

    public:
        DescriptorPools() = default;
        ~DescriptorPools();

        void Initialize(std::shared_ptr<VKContext> context);
        void Free();

        std::unique_ptr<Descriptor>
        AllocDescriptor(const VkDescriptorSetLayout& descriptor_set_layout);

        /**
         * @brief: Reset the pools to start looking for free space from the first descriptor pool.
         */
        void Reset();

    private:
        VkDescriptorPool GetActivatedPool();
        void             ActivateNextPool();
        void             ActivateLastPool();
        bool             LastPoolActivated();
        void             AllocPool();
    };

} // namespace Chandelier