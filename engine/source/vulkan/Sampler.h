#pragma once

#include "VkCommon.h"
#include "GPUSampler.h"

namespace Chandelier
{
    class Sampler
    {
    public:
        Sampler() = default;
        ~Sampler();

        void Initialize(std::shared_ptr<VKContext> context, const GPUSamplerState& sampler_state);
        void UnInit();

        bool IsInited() const { return m_sampler != VK_NULL_HANDLE; }

        VkSampler GetSampler() const;

    private:
        std::shared_ptr<VKContext> m_context;

        VkSampler m_sampler = VK_NULL_HANDLE;
    };

    class SamplerManager
    {
    public:
        SamplerManager() = default;
        ~SamplerManager();
        
        void Initialize(std::shared_ptr<VKContext> context);
        void UnInit();

        Sampler& GetSampler(const GPUSamplerState& sampler_state);

    private:
        std::shared_ptr<VKContext> m_context;

        Sampler m_sampler_cache[GPU_SAMPLER_EXTEND_MODES_COUNT][GPU_SAMPLER_EXTEND_MODES_COUNT]
                               [GPU_SAMPLER_FILTERING_TYPES_COUNT];
    };

} // namespace Chandelier
