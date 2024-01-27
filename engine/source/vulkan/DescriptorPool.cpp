#include "DescriptorPool.h"

#include "runtime/core/base/exception.h"

#include "Descriptor.h"
#include "VkContext.h"

namespace Chandelier
{
    DescriptorPools::~DescriptorPools() { Free(); }

    void DescriptorPools::Initialize(std::shared_ptr<VKContext> context)
    {
        m_context = context;
        AllocPool();
    }

    void DescriptorPools::Free()
    {
        for (const VkDescriptorPool pool : m_pools)
        {
            vkDestroyDescriptorPool(m_context->getDevice(), pool, nullptr);
        }
    }

    std::unique_ptr<Descriptor>
    DescriptorPools::AllocDescriptor(const VkDescriptorSetLayout& descriptor_set_layout)
    {
        assert(descriptor_set_layout != VK_NULL_HANDLE);

        VkDescriptorPool pool = GetActivatedPool();

        VkDescriptorSetAllocateInfo allocate_info = {};
        allocate_info.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocate_info.descriptorPool              = pool;
        allocate_info.descriptorSetCount          = 1;
        allocate_info.pSetLayouts                 = &descriptor_set_layout;
        VkDescriptorSet vk_descriptor_set         = VK_NULL_HANDLE;

        VkResult result =
            vkAllocateDescriptorSets(m_context->getDevice(), &allocate_info, &vk_descriptor_set);

        if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
        {
            if (LastPoolActivated())
            {
                AllocPool();
                ActivateLastPool();
            }
            else
            {
                ActivateNextPool();
            }
            return AllocDescriptor(descriptor_set_layout);
        }

        return std::make_unique<Descriptor>(m_context, pool, vk_descriptor_set);
    }

    void DescriptorPools::Reset() { m_active_pool_index = 0; }

    VkDescriptorPool DescriptorPools::GetActivatedPool() { return m_pools[m_active_pool_index]; }

    void DescriptorPools::ActivateNextPool() { m_active_pool_index++; }

    void DescriptorPools::ActivateLastPool() { m_active_pool_index = m_pools.size() - 1; }

    bool DescriptorPools::LastPoolActivated()
    {
        return (m_active_pool_index == m_pools.size() - 1);
    }

    void DescriptorPools::AllocPool()
    {
        std::vector<VkDescriptorPoolSize> pool_sizes = {
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, POOL_SIZE_STORAGE_BUFFER},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, POOL_SIZE_STORAGE_IMAGE},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, POOL_SIZE_COMBINED_IMAGE_SAMPLER},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, POOL_SIZE_UNIFORM_BUFFER},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, POOL_SIZE_UNIFORM_TEXEL_BUFFER},
        };

        VkDescriptorPoolCreateInfo pool_info {};
        pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets       = POOL_SIZE_DESCRIPTOR_SETS;
        pool_info.poolSizeCount = pool_sizes.size();
        pool_info.pPoolSizes    = pool_sizes.data();

        VkDescriptorPool descriptor_pool {};

        VULKAN_API_CALL(
            vkCreateDescriptorPool(m_context->getDevice(), &pool_info, nullptr, &descriptor_pool));

        m_pools.push_back(descriptor_pool);
    }

} // namespace Chandelier