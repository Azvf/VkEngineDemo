#pragma once

#include "VkCommon.h"

namespace Chandelier
{
    class Sampler
    {
    public:
        Sampler() = default;
        ~Sampler();

        void Create(std::shared_ptr<VKContext> context);
        void Free();

        VkSampler GetSampler() const;

    private:
        std::shared_ptr<VKContext> m_context;

        VkSampler m_sampler = VK_NULL_HANDLE;
    };

} // namespace Chandelier
