#pragma once

#include <memory>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "VkCreateInfo.h"

namespace Chandelier
{
    class Descriptor
    {
    public:
        Descriptor() = default;
        ~Descriptor();

        void                               Initialize(const DescriptorCreateInfo& info);
        static std::shared_ptr<Descriptor> Create(const DescriptorCreateInfo& info);

        const VkDescriptorSet& getDescriptorSet(uint32_t index) const;
        VkDescriptorSetLayout  getDescriptorLayout() const;

    private:
        VkDescriptorSetLayout        m_descriptorSetLayout;
        std::vector<VkDescriptorSet> m_descriptorSets;

        DescriptorCreateInfo m_info;
    };
} // namespace Chandelier