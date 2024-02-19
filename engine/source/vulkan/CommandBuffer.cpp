#include "CommandBuffer.h"

#include "runtime/core/base/exception.h"

#include "VkContext.h"

namespace Chandelier
{
    CommandBuffer::~CommandBuffer() { Free(); }

    void
    CommandBuffer::Initialize(std::shared_ptr<VKContext> context, VkCommandPool pool, VkCommandBuffer buffer)
    {
        if (Valid())
            return;

        m_context      = context;
        m_command_pool = pool;
        m_buffer       = buffer;
        m_state.stage  = Stage::Initial;
    }

    void CommandBuffer::Free()
    {
        if (m_buffer != VK_NULL_HANDLE)
        {
            const auto& device = m_context->getDevice();
            vkFreeCommandBuffers(device, m_command_pool, 1, &m_buffer);
            m_buffer = VK_NULL_HANDLE;
        }
        m_command_pool = VK_NULL_HANDLE;
    }

    void CommandBuffer::BeginRecording()
    {
        if (IsInStage(Stage::Submitted))
        {
            TransferStage(Stage::Submitted, Stage::Executed);
        }
        if (IsInStage(Stage::Executed))
        {
            VULKAN_API_CALL(vkResetCommandBuffer(m_buffer, 0));
            TransferStage(Stage::Executed, Stage::Initial);
        }

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VULKAN_API_CALL(vkBeginCommandBuffer(m_buffer, &begin_info));

        TransferStage(Stage::Initial, Stage::Recording);
        m_state.recorded_command_counts = 0;
    }

    void CommandBuffer::EndRecording()
    {
        VULKAN_API_CALL(vkEndCommandBuffer(m_buffer));
        TransferStage(Stage::Recording, Stage::BetweenRecordingAndSubmitting);
    }

    void CommandBuffer::CommandsSubmitted()
    {
        TransferStage(Stage::BetweenRecordingAndSubmitting, Stage::Submitted);
    }

    bool CommandBuffer::Valid() { return m_buffer != VK_NULL_HANDLE; }

} // namespace Chandelier