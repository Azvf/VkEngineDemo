#pragma once

#include <cassert>
#include <cstdint>
#include <memory>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Chandelier
{
    class VKContext;
    class TimelineSemaphore
    {
    public:
        /**
         * TimelineSemaphore::Value is used to track the timeline semaphore value.
         */
        class Value
        {
            uint64_t value_ = 0;

        public:
            operator const uint64_t*() const { return &value_; }

            bool operator<(const Value& other) const { return this->value_ < other.value_; }

            bool operator==(const Value& other) const { return this->value_ == other.value_; }

        private:
            void reset() { value_ = 0; }

            void increase() { value_++; }

            friend class TimelineSemaphore;
        };

    private:
        std::shared_ptr<VKContext> m_context;

        VkSemaphore m_semaphore = VK_NULL_HANDLE;
        Value       m_value;
        Value       m_last_completed_value;

    public:
        TimelineSemaphore() = default;
        ~TimelineSemaphore();

        void Init(std::shared_ptr<VKContext> context);
        void Free();

        /**
         * Wait for semaphore completion.
         *
         * Ensuring all commands queues before and including the given value have been finished.
         */
        void Wait(const Value& value);

        Value IncreaseValue();
        Value GetValue() const;
        Value GetLastCompletedValue() const;

        VkSemaphore GetHandle() const
        {
            assert(m_semaphore != VK_NULL_HANDLE);
            return m_semaphore;
        }
    };

} // namespace Chandelier