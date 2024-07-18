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

    Descriptor::Descriptor(const Descriptor& other) noexcept
    {
        context    = other.context;
        set_layout = other.set_layout;
        desc_set   = other.desc_set;
        desc_pool  = other.desc_pool;
        bindings   = other.bindings;
    }

    Descriptor::Descriptor(Descriptor&& other) noexcept
    {
        context    = other.context;
        set_layout = other.set_layout;
        desc_set   = other.desc_set;
        desc_pool  = other.desc_pool;
        bindings   = other.bindings;

        other.context    = nullptr;
        other.set_layout = VK_NULL_HANDLE;
        other.desc_set   = VK_NULL_HANDLE;
        other.desc_pool  = VK_NULL_HANDLE;
        other.bindings.clear();
    }

    Descriptor& Descriptor::operator=(const Descriptor& other) noexcept 
    {
        context    = other.context;
        set_layout = other.set_layout;
        desc_set   = other.desc_set;
        desc_pool  = other.desc_pool;
        bindings   = other.bindings;

        return *this;
    }

    Descriptor& Descriptor::operator=(Descriptor&& other) noexcept
    {
        context    = other.context;
        set_layout = other.set_layout;
        desc_set   = other.desc_set;
        desc_pool  = other.desc_pool;
        bindings   = other.bindings;

        other.context    = nullptr;
        other.set_layout = VK_NULL_HANDLE;
        other.desc_set   = VK_NULL_HANDLE;
        other.desc_pool  = VK_NULL_HANDLE;
        other.bindings.clear();

        return *this;
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
        if (texture->GetTextureType() == Render_Target_Texture)
        {
            binding.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        }
        else
        {
            assert(0);
            binding.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
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
            binding.texture->TransferLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            
            bool is_attachment_image = (binding.type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
            
            VkDescriptorImageInfo image_info = {};
            image_info.sampler               = is_attachment_image ? VK_NULL_HANDLE : binding.vk_sampler;
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

        assert(descriptor_writes.size() == m_bindings.size());
        vkUpdateDescriptorSets(
            m_context->getDevice(), descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);

        m_bindings.clear();
    }

    BindTable::~BindTable() { UnInit(); }

    VkDescriptorSetLayoutBinding BindTable::CreateLayoutBinding(const Binding& binding) 
    {
        VkDescriptorSetLayoutBinding layout_binding = {};
        layout_binding.binding                      = binding.location.binding;
        layout_binding.descriptorType               = binding.type;
        layout_binding.descriptorCount              = 1;
        layout_binding.stageFlags                   = binding.shader_stages;
        layout_binding.pImmutableSamplers           = nullptr;

        return layout_binding;
    }

    VkDescriptorSetLayout BindTable::CreateLayout(VkDevice device, const std::vector<Binding>& bindings) 
    {
        VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;

        std::vector<VkDescriptorSetLayoutBinding> binding_layout;

        for (const auto& binding : bindings)
        {
            binding_layout.push_back(BindTable::CreateLayoutBinding(binding));
        }

        VkDescriptorSetLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.bindingCount                    = binding_layout.size();
        layout_create_info.pBindings                       = binding_layout.data();
        VULKAN_API_CALL(vkCreateDescriptorSetLayout(device, &layout_create_info, nullptr, &set_layout));

        return set_layout;
    }

    void BindTable::Initialize(const BindTableCI& ci) { 
        // m_descriptors.resize(ci.descriptor_size);
        m_descriptors.resize(1); 
    }

    void BindTable::UnInit() 
    {
        auto& pool = m_context->GetDescriptorPools();
        
        std::vector<VkDescriptorSet> sets_to_free;
        for (auto& descriptor : m_descriptors)
        {
            if (auto handle = descriptor.Handle())
            {
                sets_to_free.push_back(handle);
            }
        }
        
        VULKAN_API_CALL(
            vkFreeDescriptorSets(m_context->getDevice(), pool.Handle(), sets_to_free.size(), sets_to_free.data()));
        
        for (auto& descriptor : m_descriptors)
        {
            if (!descriptor.set_layout)
            {
                continue;
            }

            vkDestroyDescriptorSetLayout(m_context->getDevice(), descriptor.set_layout, nullptr);
        }

    }

    void BindTable::Sync()
    {
        for (auto& descriptor : m_descriptors)
        {
            if (descriptor.bindings.empty())
            {
                continue;
            }

            if (!descriptor.desc_set)
            {
                auto layout           = BindTable::CreateLayout(m_context->getDevice(), descriptor.bindings);
                descriptor.set_layout = layout;
                auto& pool            = m_context->GetDescriptorPools();
                descriptor.desc_set   = pool.Allocate(layout);
            }

            bool update_needed = false;
            for (const auto& binding : descriptor.bindings)
            {
                if (binding.binded == false)
                {
                    update_needed = true;
                    break;
                }
            }

            if (!update_needed)
            {
                continue;
            }

            std::vector<VkDescriptorBufferInfo> buffer_infos;
            buffer_infos.reserve(16);
            std::vector<VkWriteDescriptorSet> descriptor_writes;

            for (const Binding& binding : descriptor.bindings)
            {
                if (binding.binded)
                {
                    continue;
                }

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
                write_descriptor.dstSet               = descriptor.desc_set;
                write_descriptor.dstBinding           = binding.location;
                write_descriptor.descriptorCount      = 1;
                write_descriptor.descriptorType       = binding.type;
                write_descriptor.pBufferInfo          = &buffer_infos.back();
                descriptor_writes.push_back(write_descriptor);

                const_cast<Binding&>(binding).binded = true;
            }

            for (const Binding& binding : descriptor.bindings)
            {
                if (binding.binded)
                {
                    continue;
                }

                if (!binding.is_texel_buffer())
                {
                    continue;
                }

                VkWriteDescriptorSet write_descriptor = {};
                write_descriptor.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_descriptor.dstSet               = descriptor.desc_set;
                write_descriptor.dstBinding           = binding.location;
                write_descriptor.descriptorCount      = 1;
                write_descriptor.descriptorType       = binding.type;
                write_descriptor.pTexelBufferView     = &binding.vk_buffer_view;

                descriptor_writes.push_back(write_descriptor);

                const_cast<Binding&>(binding).binded = true;
            }

            std::vector<VkDescriptorImageInfo> image_infos;
            image_infos.reserve(16);
            for (const Binding& binding : descriptor.bindings)
            {
                if (binding.binded)
                {
                    continue;
                }

                if (!binding.is_image())
                {
                    continue;
                }

                /* TODO: Based on the actual usage we should use
                 * VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL/VK_IMAGE_LAYOUT_GENERAL. */
                binding.texture->TransferLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                bool is_attachment_image = (binding.type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);

                VkDescriptorImageInfo image_info = {};
                image_info.sampler               = is_attachment_image ? VK_NULL_HANDLE : binding.vk_sampler;
                image_info.imageView             = binding.texture->getView();
                image_info.imageLayout           = binding.texture->getLayout();

                image_infos.push_back(image_info);

                VkWriteDescriptorSet write_descriptor = {};
                write_descriptor.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_descriptor.dstSet               = descriptor.desc_set;
                write_descriptor.dstBinding           = binding.location;
                write_descriptor.descriptorCount      = 1;
                write_descriptor.descriptorType       = binding.type;
                write_descriptor.pImageInfo           = &image_infos.back();

                descriptor_writes.push_back(write_descriptor);

                const_cast<Binding&>(binding).binded = true;
            }

            assert(descriptor_writes.size() == descriptor.bindings.size());
            vkUpdateDescriptorSets(
                m_context->getDevice(), descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
        }
    }
    
    Binding& BindTable::GetBinding(Location loc) 
    {
        for (auto& descriptor : m_descriptors)
        {
            for (auto& binding : descriptor.bindings)
            {
                if (binding.location == loc)
                {
                    return binding;
                }
            }
        }
        
        auto& set = m_descriptors[loc.set];

        if (set.set_layout)
        {
            std::runtime_error("requested binding layout already created");
        }

        Binding binding;
        binding.location = loc;
        
        set.bindings.push_back(binding);

        return set.bindings.back();
    }

    void BindTable::Bind(Buffer* buffer, Location loc, VkShaderStageFlags stages) 
    {
        Binding& binding = GetBinding(loc);

        binding.binded = false;

        binding.type        = buffer->GetBindType();
        binding.vk_buffer   = buffer->getBuffer();
        binding.buffer_size = buffer->GetBufferSize();

        binding.shader_stages = stages;
    }
    
    void BindTable::Bind(Texture* texture, Location loc, VkShaderStageFlags stages) 
    {
        Binding& binding = GetBinding(loc);

        binding.binded   = false;

        if (texture->GetTextureType() == Render_Target_Texture)
        {
            binding.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        }
        else
        {
            assert(0);
            binding.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        binding.texture = texture;

        binding.shader_stages = stages;
    }

    void BindTable::Bind(Texture* texture, Sampler* sampler, Location loc, VkShaderStageFlags stages) 
    {
        Binding& binding = GetBinding(loc);

        binding.binded = false;

        binding.type       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.texture    = texture;
        binding.vk_sampler = sampler->GetSampler();

        binding.shader_stages = stages;
    }

    void BindTable::BindSet(const VkPipelineLayout pipeline_layout, VkPipelineBindPoint pipeline_bind_point)
    {
        auto& command_manager = m_context->GetCommandManager();
        for (auto& descriptor : m_descriptors)
        {
            command_manager.BindDescriptorSet(descriptor.Handle(), pipeline_layout, pipeline_bind_point);
        }
    }

    void MergedBindTable::Merge(std::vector<BindTable> bind_tables) { 
        m_results.clear();

        const int end_index = m_set_count;
        const int overlap_index = INT32_MAX;
        for (int set_index = 0; set_index < m_set_count; set_index++) 
        {
            int source_table = end_index;
            for (int i = 0; i < bind_tables.size(); i++)
            {
                if (bind_tables[i].GetDescriptor(set_index).Valid())
                {
                    if (source_table == end_index)
                    {
                        source_table = i;
                    }
                    else
                    {
                        source_table = overlap_index;
                        break;
                    }
                }
            }

            if (source_table == end_index)
            {
                // do nothing
            }
            else if (source_table == overlap_index)
            {
                /*if (!m_merged[set_index].Valid())
                {
                    Descriptor set;
                    auto       layout = BindTable::CreateLayout(m_context->getDevice(),
                                                          m_bind_table_ref.GetDescriptor(set_index).bindings);
                    set.set_layout    = layout;
                    auto& pool          = m_context->GetDescriptorPools();
                    set.desc_set = pool.Allocate(layout);
                    m_merged[set_index] = set;
                }

                vkUpdateDescriptorSets(m_context->getDevice(), );
                
                m_results[set_index] = m_merged[set_index];*/
            }
            else
            {
                m_copied[set_index]  = bind_tables[source_table].GetDescriptor(set_index);
                m_results[set_index] = m_copied[set_index];
            }
        }
    }

    void MergedBindTable::BindSet(const VkPipelineLayout pipeline_layout, VkPipelineBindPoint pipeline_bind_point)
    {
        auto& command_manager = m_context->GetCommandManager();
        for (auto& descriptor : m_results)
        {
            command_manager.BindDescriptorSet(descriptor.Handle(), pipeline_layout, pipeline_bind_point);
        }
    }

    MergedBindTable::~MergedBindTable() {
        

    }


} // namespace Chandelier