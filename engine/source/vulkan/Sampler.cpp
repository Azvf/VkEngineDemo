#include "Sampler.h"

#include <iostream>

#include "runtime/core/base/exception.h"

#include "VkContext.h"
#include "VkUtil.h"

namespace Chandelier
{
    Sampler::~Sampler() { Free(); }

    void Sampler::Free()
    {
        if (m_sampler != VK_NULL_HANDLE)
        {
            const auto& device = m_context->getDevice();
            vkDestroySampler(device, m_sampler, nullptr);
        }
        m_sampler = nullptr;
    }

    void Sampler::Create(std::shared_ptr<VKContext> context)
    {
        m_context = context;

        VkSamplerCreateInfo sampler_info = {};
        sampler_info.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        const auto& device = context->getDevice();
        VULKAN_API_CALL(vkCreateSampler(device, &sampler_info, nullptr, &m_sampler));
    }

    VkSampler Sampler::GetSampler() const { return m_sampler; }

} // namespace Chandelier