#include "Descriptor.h"

#include <iostream>

#include "runtime/core/base/exception.h"

#include "RenderCfg.h"
#include "Sampler.h"
#include "Texture.h"
#include "Uniform.h"
#include "VkContext.h"
#include "VkUtil.h"

namespace Chandelier
{
    Descriptor::~Descriptor()
    {
        if (m_info.context)
        {
            vkDestroyDescriptorSetLayout(m_info.context->getDevice(), m_descriptorSetLayout, nullptr);
        }
    }

    std::shared_ptr<Descriptor> Descriptor::Create(const DescriptorCreateInfo& info)
    {
        auto descriptor = std::make_shared<Descriptor>();
        descriptor->Initialize(info);
        return descriptor;
    }

    void Descriptor::Initialize(const DescriptorCreateInfo& info)
    {
        const auto& device    = info.context->getDevice();
        const auto& dscp_pool = info.context->getDescriptorPool();

        VkDescriptorSetLayoutBinding ubo_layout {};
        ubo_layout.binding            = 0;
        ubo_layout.descriptorCount    = 1;
        ubo_layout.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout.pImmutableSamplers = nullptr;
        ubo_layout.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding sampler_layout {};
        sampler_layout.binding            = 1;
        sampler_layout.descriptorCount    = 1;
        sampler_layout.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_layout.pImmutableSamplers = nullptr;
        sampler_layout.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ubo_layout, sampler_layout};

        VkDescriptorSetLayoutCreateInfo layoutInfo {};
        layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings    = bindings.data();

        VULKAN_API_CALL(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout));

        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo        allocInfo {};
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = dscp_pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts        = layouts.data();

        m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        VULKAN_API_CALL(vkAllocateDescriptorSets(device, &allocInfo, m_descriptorSets.data()));

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::array<VkWriteDescriptorSet, 2> descriptorWrites {};

            VkDescriptorBufferInfo bufferInfo {};
            bufferInfo.buffer = info.uniform->getUniformBuffer(i);
            bufferInfo.offset = 0;
            bufferInfo.range  = sizeof(UniformBufferObject);

            descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet          = m_descriptorSets[i];
            descriptorWrites[0].dstBinding      = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo     = &bufferInfo;

            VkDescriptorImageInfo imageInfo {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView   = info.texture->getView();
            imageInfo.sampler     = info.sampler->getTextureSampler();

            descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet          = m_descriptorSets[i];
            descriptorWrites[1].dstBinding      = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo      = &imageInfo;

            vkUpdateDescriptorSets(
                device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        m_info = info;
    }

    const VkDescriptorSet& Descriptor::getDescriptorSet(uint32_t index) const { return m_descriptorSets[index]; }

    VkDescriptorSetLayout Descriptor::getDescriptorLayout() const { return m_descriptorSetLayout; }

} // namespace Chandelier