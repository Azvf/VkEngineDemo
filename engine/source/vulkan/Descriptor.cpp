#include "Descriptor.h"

#include <iostream>

#include "runtime/core/base/base_utility.h"
#include "runtime/core/base/exception.h"

#include "Buffer.h"
#include "Sampler.h"
#include "Texture.h"
#include "Uniform.h"
#include "VkContext.h"
#include "VkUtil.h"
#include "DescriptorPool.h"

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

    Binding& DescriptorTracker::EnsureLocation(Location loc)
    {
        for (auto& binding : m_bindings)
        {
            if (binding.location == loc)
            {
                return binding;
            }
        }

        Binding binding;
        binding.location = loc;
        m_bindings.push_back(binding);

        return m_bindings.back();
    }

    Binding& DescriptorTracker::GetBinding(Location loc)
    {
        return m_bindings.at(loc.binding);
    }

    void DescriptorTracker::Bind(Buffer* buffer, Location loc, VkShaderStageFlags stages)
    {
        Binding& binding = EnsureLocation(loc);

        binding.type        = buffer->GetBindType();
        binding.vk_buffer   = buffer->getBuffer();
        binding.buffer_size = buffer->GetBufferSize();

        binding.shader_stages = stages;
    }

    void DescriptorTracker::Bind(Texture* texture, Location loc, VkShaderStageFlags stages)
    {
        Binding& binding = EnsureLocation(loc);

        binding.type    = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        binding.texture = texture;
        
        binding.shader_stages = stages;
    }

    void DescriptorTracker::Bind(Texture* texture, Sampler* sampler, Location loc, VkShaderStageFlags stages)
    {
        Binding& binding = EnsureLocation(loc);

        binding.type       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.texture    = texture;
        binding.vk_sampler = sampler->GetSampler();

        binding.shader_stages = stages;
    }

    void DescriptorTracker::BindDescriptorSet(const VkPipelineLayout pipeline_layout,
                                              VkPipelineBindPoint    pipeline_bind_point)
    {
        auto& command_manager = m_context->GetCommandManager();
        command_manager.BindDescriptorSet(GetResource()->Handle(), pipeline_layout, pipeline_bind_point);
    }

    VkDescriptorSetLayoutBinding DescriptorTracker::CreateLayoutBinding(const Binding& binding)
    {
        VkDescriptorSetLayoutBinding layout_binding = {};
        layout_binding.binding                      = binding.location.binding;
        layout_binding.descriptorType               = binding.type;
        layout_binding.descriptorCount              = 1;
        layout_binding.stageFlags                   = binding.shader_stages;
        layout_binding.pImmutableSamplers           = nullptr;

        return layout_binding;
    }

    VkDescriptorSetLayout DescriptorTracker::CreateLayout()
    {
        VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;

        std::vector<VkDescriptorSetLayoutBinding> vk_set_layout_bindings;

        for (auto& binding : m_bindings)
        {
            vk_set_layout_bindings.push_back(DescriptorTracker::CreateLayoutBinding(binding));
        }

        VkDescriptorSetLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.bindingCount = vk_set_layout_bindings.size();
        layout_create_info.pBindings    = vk_set_layout_bindings.data();
        VULKAN_API_CALL(vkCreateDescriptorSetLayout(m_context->getDevice(), &layout_create_info, nullptr, &set_layout));

        return set_layout;
    }

    void DescriptorTracker::Sync(/*VkDescriptorSetLayout new_layout*/)
    {
        // bool  dirty      = !m_bindings.empty() || AssignIfDiff(m_active_desc_layout, new_layout);
        bool dirty = !m_bindings.empty();

        if (dirty)
        {
            m_active_desc_layout = CreateLayout();
        }

        auto& descriptor = UpdateResources(m_context.get(), dirty);
        VkDescriptorSet dst_set = descriptor->Handle();

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

    std::shared_ptr<Descriptor> DescriptorTracker::CreateResource()
    {
        return m_context->GetDescriptorPools().AllocDescriptor(m_active_desc_layout);
    }

} // namespace Chandelier