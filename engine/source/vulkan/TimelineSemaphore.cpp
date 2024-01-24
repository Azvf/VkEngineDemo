#include "TimelineSemaphore.h"

#include "VkContext.h"

namespace Chandelier
{
    TimelineSemaphore::~TimelineSemaphore()
    {
        Free();
    }

    void TimelineSemaphore::Init(std::shared_ptr<VKContext> context)
    {
        if (m_semaphore != VK_NULL_HANDLE)
            return;

        m_context = context;

        static constexpr const VkAllocationCallbacks* vk_allocation_callbacks = nullptr;
        VkSemaphoreTypeCreateInfo semaphore_type_create_info = {};
        semaphore_type_create_info.sType                     = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        semaphore_type_create_info.semaphoreType             = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphore_type_create_info.initialValue              = 0;

        VkSemaphoreCreateInfo semaphore_create_info {};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_create_info.pNext = &semaphore_type_create_info;
        vkCreateSemaphore(
            context->getDevice(), &semaphore_create_info, vk_allocation_callbacks, &m_semaphore);

        m_value.reset();
    }

    void TimelineSemaphore::Free()
    {
        if (m_semaphore == VK_NULL_HANDLE)
            return;

        static constexpr const VkAllocationCallbacks* vk_allocation_callbacks = nullptr;
        vkDestroySemaphore(m_context->getDevice(), m_semaphore, vk_allocation_callbacks);
        m_semaphore = VK_NULL_HANDLE;

        m_value.reset();
    }

    void TimelineSemaphore::Wait(const Value& wait_value)
    {
        assert(m_semaphore != VK_NULL_HANDLE);

        VkSemaphoreWaitInfo wait_info = {};
        wait_info.sType               = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        wait_info.semaphoreCount      = 1;
        wait_info.pSemaphores         = &m_semaphore;
        wait_info.pValues             = wait_value;
        vkWaitSemaphores(m_context->getDevice(), &wait_info, UINT64_MAX);
        m_last_completed_value = wait_value;
    }

    TimelineSemaphore::Value TimelineSemaphore::IncreaseValue()
    {
        m_value.increase();
        return m_value;
    }

    TimelineSemaphore::Value TimelineSemaphore::GetValue() const { return m_value; }

    TimelineSemaphore::Value TimelineSemaphore::GetLastCompletedValue() const
    {
        return m_last_completed_value;
    }


}