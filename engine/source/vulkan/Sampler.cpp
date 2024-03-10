#include "Sampler.h"

#include <iostream>

#include "runtime/core/base/exception.h"

#include "VkContext.h"
#include "VkUtil.h"

namespace Chandelier
{
    Sampler::~Sampler() { UnInit(); }

    void Sampler::UnInit()
    {
        if (m_sampler != VK_NULL_HANDLE)
        {
            const auto& device = m_context->getDevice();
            vkDestroySampler(device, m_sampler, nullptr);
        }
        m_sampler = nullptr;
    }

    void Sampler::Initialize(std::shared_ptr<VKContext> context, const GPUSamplerState& sampler_state)
    {
        m_context = context;

        VkSamplerCreateInfo sampler_info = {};
        sampler_info.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        /* Extend */
        sampler_info.addressModeU = CvtToVkSamplerAddrMode(sampler_state.extend_x);
        sampler_info.addressModeV = sampler_info.addressModeW = CvtToVkSamplerAddrMode(sampler_state.extend_yz);
        sampler_info.minLod                                   = 0;
        sampler_info.maxLod                                   = 1000;

        if (sampler_state.type == GPU_SAMPLER_STATE_TYPE_PARAMETERS)
        {
            /* Apply filtering. */
            if (sampler_state.filtering & GPU_SAMPLER_FILTERING_LINEAR)
            {
                sampler_info.magFilter = VK_FILTER_LINEAR;
                sampler_info.minFilter = VK_FILTER_LINEAR;
            }
            if (sampler_state.filtering & GPU_SAMPLER_FILTERING_MIPMAP)
            {
                sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            }
            if ((sampler_state.filtering & GPU_SAMPLER_FILTERING_ANISOTROPIC) &&
                (context->getDeviceFeatures().features.samplerAnisotropy == VK_TRUE))
            {
                sampler_info.anisotropyEnable = VK_TRUE;
                sampler_info.maxAnisotropy    = 16.0f;
            }
        }
        else if (sampler_state.type == GPU_SAMPLER_STATE_TYPE_CUSTOM)
        {
            if (sampler_state.custom_type == GPU_SAMPLER_CUSTOM_ICON)
            {
                sampler_info.magFilter  = VK_FILTER_LINEAR;
                sampler_info.minFilter  = VK_FILTER_LINEAR;
                sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                sampler_info.minLod     = 0;
                sampler_info.maxLod     = 1;
            }
            else if (sampler_state.custom_type == GPU_SAMPLER_CUSTOM_COMPARE)
            {
                sampler_info.magFilter     = VK_FILTER_LINEAR;
                sampler_info.minFilter     = VK_FILTER_LINEAR;
                sampler_info.compareEnable = VK_TRUE;
                sampler_info.compareOp     = VK_COMPARE_OP_LESS_OR_EQUAL;
            }
        }

        VULKAN_API_CALL(vkCreateSampler(context->getDevice(), &sampler_info, nullptr, &m_sampler));
    }

    VkSampler Sampler::GetSampler() const { return m_sampler; }

    SamplerManager::~SamplerManager() { UnInit(); }

    void SamplerManager::Initialize(std::shared_ptr<VKContext> context)
    {
        m_context = context;

        GPUSamplerState state {};

        for (int extend_yz = 0; extend_yz < GPU_SAMPLER_EXTEND_MODES_COUNT; extend_yz++)
        {
            state.extend_yz = static_cast<GPUSamplerExtendMode>(extend_yz);
            for (int extend_x = 0; extend_x < GPU_SAMPLER_EXTEND_MODES_COUNT; extend_x++)
            {
                state.extend_x = static_cast<GPUSamplerExtendMode>(extend_x);
                for (int filtering = 0; filtering < GPU_SAMPLER_FILTERING_TYPES_COUNT; filtering++)
                {
                    /**
                     * @todo: is the enum conversion valid? 
                     */
                    state.filtering = static_cast<GPUSamplerFiltering>(filtering);
                    m_sampler_cache[extend_yz][extend_x][filtering].Initialize(context, state);
                }
            }
        }
    }

    void SamplerManager::UnInit()
    {
        for (int extend_yz = 0; extend_yz < GPU_SAMPLER_EXTEND_MODES_COUNT; extend_yz++)
        {
            for (int extend_x = 0; extend_x < GPU_SAMPLER_EXTEND_MODES_COUNT; extend_x++)
            {
                for (int filtering = 0; filtering < GPU_SAMPLER_FILTERING_TYPES_COUNT; filtering++)
                {
                    m_sampler_cache[extend_yz][extend_x][filtering].UnInit();
                }
            }
        }
    }

    Sampler& SamplerManager::GetSampler(const GPUSamplerState& sampler_state) {
        return m_sampler_cache[sampler_state.extend_yz][sampler_state.extend_x][sampler_state.filtering];
    }

} // namespace Chandelier