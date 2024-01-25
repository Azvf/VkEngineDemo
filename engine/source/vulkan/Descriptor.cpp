#include "Descriptor.h"

#include <iostream>

#include "runtime/core/base/exception.h"

#include "Buffer.h"
#include "RenderCfg.h"
#include "Sampler.h"
#include "Texture.h"
#include "Uniform.h"
#include "VkContext.h"
#include "VkUtil.h"

namespace Chandelier
{
    Descriptor::Descriptor(Descriptor&& other) :
        m_context(other.m_context), m_desc_pool(other.m_desc_pool), m_desc_set(other.m_desc_set)
    {
        other.m_context   = nullptr;
        other.m_desc_set  = VK_NULL_HANDLE;
        other.m_desc_pool = VK_NULL_HANDLE;
    }

    Descriptor::~Descriptor()
    {
        if (m_desc_set != VK_NULL_HANDLE)
        {
            vkFreeDescriptorSets(m_context->getDevice(), m_desc_pool, 1, &m_desc_set);

            m_desc_set  = VK_NULL_HANDLE;
            m_desc_pool = VK_NULL_HANDLE;
        }
    }

    DescriptorTracker::~DescriptorTracker() {}

    void DescriptorTracker::Bind(std::unique_ptr<Buffer> buffer, Location loc) {}

    void DescriptorTracker::Reset() {}

    void DescriptorTracker::Sync()
    {
        // todo: improve dirty falg situation

        bool            dirty      = !m_bindings.empty();
        auto&           descriptor = UpdateResources(m_context.get(), dirty);
        VkDescriptorSet dst_set    = descriptor->Handle();

        std::vector<VkDescriptorBufferInfo> buffer_infos;
        buffer_infos.reserve(16);
        std::vector<VkWriteDescriptorSet> descriptor_writes;

        for (const Binding& binding : m_bindings)
        {
            if (!binding.is_buffer())
            {
                continue;
            }
            VkDescriptorBufferInfo buffer_info = {};
            buffer_info.buffer                 = binding.vk_buffer;
            buffer_info.range                  = binding.buffer_size;
            buffer_infos.push_back(buffer_info);

            VkWriteDescriptorSet write_descriptor = {};
            write_descriptor.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor.dstSet               = dst_set;
            write_descriptor.dstBinding           = binding.location;
            write_descriptor.descriptorCount      = 1;
            write_descriptor.descriptorType       = binding.type;
            write_descriptor.pBufferInfo          = &buffer_infos.back();
            descriptor_writes.push_back(write_descriptor);
        }

        for (const Binding& binding : m_bindings)
        {
            if (!binding.is_texel_buffer())
            {
                continue;
            }
            VkWriteDescriptorSet write_descriptor = {};
            write_descriptor.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor.dstSet               = dst_set;
            write_descriptor.dstBinding           = binding.location;
            write_descriptor.descriptorCount      = 1;
            write_descriptor.descriptorType       = binding.type;
            write_descriptor.pTexelBufferView     = &binding.vk_buffer_view;

            descriptor_writes.push_back(write_descriptor);
        }

        std::vector<VkDescriptorImageInfo> image_infos;
        image_infos.reserve(16);
        for (const Binding& binding : m_bindings)
        {
            if (!binding.is_image())
            {
                continue;
            }

            /* TODO: Based on the actual usage we should use
             * VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL/VK_IMAGE_LAYOUT_GENERAL. */
            binding.texture->TransferLayout(VK_IMAGE_LAYOUT_GENERAL);
            VkDescriptorImageInfo image_info = {};
            image_info.sampler               = binding.vk_sampler;
            image_info.imageView             = binding.texture->getView();
            image_info.imageLayout           = binding.texture->getLayout();

            image_infos.push_back(image_info);

            VkWriteDescriptorSet write_descriptor = {};
            write_descriptor.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor.dstSet               = dst_set;
            write_descriptor.dstBinding           = binding.location;
            write_descriptor.descriptorCount      = 1;
            write_descriptor.descriptorType       = binding.type;
            write_descriptor.pImageInfo           = &image_infos.back();

            descriptor_writes.push_back(write_descriptor);
        }

        vkUpdateDescriptorSets(
            m_context->getDevice(), descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);

        m_bindings.clear();
    }

    std::unique_ptr<Descriptor> DescriptorTracker::CreateResource()
    {
        return m_context->GetDescriptorPools().AllocDescriptor(m_active_desc_layout);
    }

} // namespace Chandelier