#include "CommandBuffers.h"

#include <cassert>

#include "runtime/core/base/exception.h"

#include "Buffer.h"
#include "Texture.h"
#include "VkContext.h"

namespace Chandelier
{
    CommandBuffers::~CommandBuffers() { Free(); }

    bool CommandBuffers::Valid() { return m_valid; }

    void CommandBuffers::Initialize(std::shared_ptr<VKContext> context)
    {
        if (!context)
            return;

        m_context = context;
        assert(m_command_pool == VK_NULL_HANDLE);

        static constexpr const VkAllocationCallbacks* vk_allocation_callbacks = nullptr;

        VkCommandPoolCreateInfo command_pool_info = {};
        command_pool_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_info.queueFamilyIndex        = context->getGraphicsQueueFamilyIndex();

        VULKAN_API_CALL(vkCreateCommandPool(
            context->getDevice(), &command_pool_info, vk_allocation_callbacks, &m_command_pool));

        VkCommandBuffer             vk_command_buffers[Type::Max] = {VK_NULL_HANDLE};
        VkCommandBufferAllocateInfo alloc_info                    = {};
        alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool        = m_command_pool;
        alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = uint32_t(Type::Max);
        vkAllocateCommandBuffers(m_context->getDevice(), &alloc_info, vk_command_buffers);

        InitCommandBuffer(GetCommandBuffer(Type::DataTransferCompute),
                          m_command_pool,
                          vk_command_buffers[Type::DataTransferCompute]);
        InitCommandBuffer(
            GetCommandBuffer(Type::Graphics), m_command_pool, vk_command_buffers[Type::Graphics]);

        m_subid.reset();
        m_valid = true;
    }

    void CommandBuffers::Free()
    {
        GetCommandBuffer(Type::DataTransferCompute).Free();
        GetCommandBuffer(Type::Graphics).Free();

        const auto& device = m_context->getDevice();

        static constexpr const VkAllocationCallbacks* vk_allocation_callbacks = nullptr;
        if (m_command_pool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device, m_command_pool, vk_allocation_callbacks);
            m_command_pool = VK_NULL_HANDLE;
        }
    }

    void CommandBuffers::InitCommandBuffer(CommandBuffer&  command_buffer,
                                           VkCommandPool   pool,
                                           VkCommandBuffer vk_command_buffer)
    {
        command_buffer.Initialize(m_context, pool, vk_command_buffer);
        command_buffer.BeginRecording();
    }

    void
    CommandBuffers::IssuePipelineBarrier(const VkPipelineStageFlags        src_stages,
                                         const VkPipelineStageFlags        dst_stages,
                                         std::vector<VkImageMemoryBarrier> image_memory_barriers)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(Type::DataTransferCompute);
        vkCmdPipelineBarrier(command_buffer.GetCommandBuffer(),
                             src_stages,
                             dst_stages,
                             VK_DEPENDENCY_BY_REGION_BIT,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             image_memory_barriers.size(),
                             image_memory_barriers.data());
        command_buffer.CommandRecorded();
    }

    void CommandBuffers::ClearColorAttachment(VkImage                              vk_image,
                                              VkImageLayout                        vk_image_layout,
                                              const VkClearColorValue&             vk_clear_color,
                                              std::vector<VkImageSubresourceRange> ranges)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(Type::DataTransferCompute);
        vkCmdClearColorImage(command_buffer.GetCommandBuffer(),
                             vk_image,
                             vk_image_layout,
                             &vk_clear_color,
                             ranges.size(),
                             ranges.data());
        command_buffer.CommandRecorded();
    }

    void CommandBuffers::ClearDepthStencil(VkImage                         vk_image,
                                           VkImageLayout                   vk_image_layout,
                                           const VkClearDepthStencilValue& vk_clear_depth_stencil,
                                           std::vector<VkImageSubresourceRange> ranges)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(Type::DataTransferCompute);
        vkCmdClearDepthStencilImage(command_buffer.GetCommandBuffer(),
                                    vk_image,
                                    vk_image_layout,
                                    &vk_clear_depth_stencil,
                                    ranges.size(),
                                    ranges.data());
        command_buffer.CommandRecorded();
    }

    void CommandBuffers::SubmitCommandBuffers(const std::vector<CommandBuffer*>& command_buffers)
    {
        VkSemaphore              timeline_handle = m_semaphore.GetHandle();
        TimelineSemaphore::Value wait_value      = m_semaphore.GetValue();
        m_last_signal_value                      = m_semaphore.IncreaseValue();

        VkCommandBuffer handles[2];
        int             num_command_buffers = 0;

        for (CommandBuffer* command_buffer : command_buffers)
        {
            command_buffer->EndRecording();
            handles[num_command_buffers++] = command_buffer->GetCommandBuffer();
        }

        VkTimelineSemaphoreSubmitInfo timelineInfo;
        timelineInfo.sType                     = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineInfo.pNext                     = nullptr;
        timelineInfo.waitSemaphoreValueCount   = 1;
        timelineInfo.pWaitSemaphoreValues      = wait_value;
        timelineInfo.signalSemaphoreValueCount = 1;
        timelineInfo.pSignalSemaphoreValues    = m_last_signal_value;

        VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkSubmitInfo         submit_info = {};
        submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount   = num_command_buffers;
        submit_info.pCommandBuffers      = handles;
        submit_info.pNext                = &timelineInfo;
        submit_info.waitSemaphoreCount   = 1;
        submit_info.pWaitSemaphores      = &timeline_handle;
        submit_info.pWaitDstStageMask    = &wait_stages;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores    = &timeline_handle;

        vkQueueSubmit(m_context->getGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
        Wait();

        for (CommandBuffer* command_buffer : command_buffers)
        {
            command_buffer->CommandsSubmitted();
            command_buffer->BeginRecording();
        }
    }

    void CommandBuffers::EnsureNoDrawCommands()
    {
        if (GetCommandBuffer(Graphics).HasRecordedCommands())
        {
            Submit();
        }
    }

    void CommandBuffers::Wait()
    {
        m_semaphore.Wait(m_last_signal_value);
        m_subid.next();
    }

    void CommandBuffers::Submit()
    {
        const auto& device                = m_context->getDevice();
        auto&       data_transfer_compute = GetCommandBuffer(Type::DataTransferCompute);
        auto&       graphics              = GetCommandBuffer(Type::Graphics);

        const bool has_data_transfer_compute_work = data_transfer_compute.HasRecordedCommands();
        const bool has_graphics_work              = graphics.HasRecordedCommands();

        CommandBuffer* command_buffers[2]   = {nullptr, nullptr};
        int            command_buffer_index = 0;

        if (has_data_transfer_compute_work)
        {
            command_buffers[command_buffer_index++] = &data_transfer_compute;
        }

        if (has_graphics_work)
        {
            // VKFrameBuffer* framebuffer = framebuffer_;
            // end_render_pass(*framebuffer);
            command_buffers[command_buffer_index++] = &graphics;
        }

        SubmitCommandBuffers(
            std::vector<CommandBuffer*>(command_buffers, command_buffers + command_buffer_index));

        if (has_graphics_work)
        {
            // begin_render_pass(*framebuffer);
        }
    }

    void CommandBuffers::Copy(std::shared_ptr<Buffer>  src_buffer,
                              std::shared_ptr<Texture> dst_texture,
                              const std::vector<VkBufferImageCopy>& regions)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(Type::DataTransferCompute);
        vkCmdCopyBufferToImage(command_buffer.GetCommandBuffer(),
                               src_buffer->getBuffer(),
                               dst_texture->getImage(),
                               dst_texture->getLayout(),
                               regions.size(),
                               regions.data());
        command_buffer.CommandRecorded();
    }

    CommandBuffer& CommandBuffers::GetCommandBuffer(Type type) { return m_buffers[type]; }
    
    SubmissionID&  CommandBuffers::GetSubmissionId() { return m_subid; }


} // namespace Chandelier