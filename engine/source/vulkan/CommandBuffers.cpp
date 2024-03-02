#include "CommandBuffers.h"

#include <cassert>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include "runtime/core/base/exception.h"

#include "VkContext.h"
#include "Buffer.h"
#include "Texture.h"
#include "Mesh.h"

namespace Chandelier
{
    CommandBufferManager::~CommandBufferManager() { UnInit(); }

    bool CommandBufferManager::Valid() { return m_valid; }

    void CommandBufferManager::Initialize(std::shared_ptr<VKContext> context)
    {
        if (m_valid)
            return;

        m_context = context;
        assert(m_command_pool == VK_NULL_HANDLE);

        VkCommandPoolCreateInfo command_pool_info = {};
        command_pool_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_info.queueFamilyIndex        = context->getGraphicsQueueFamilyIndex();

        VULKAN_API_CALL(vkCreateCommandPool(context->getDevice(), &command_pool_info, nullptr, &m_command_pool));

        VkCommandBuffer             vk_command_buffers[CommandBufferTypeCount] = {VK_NULL_HANDLE};
        VkCommandBufferAllocateInfo alloc_info                    = {};
        alloc_info.sType                                          = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool                                    = m_command_pool;
        alloc_info.level                                          = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount                             = uint32_t(CommandBufferTypeCount);
        VULKAN_API_CALL(vkAllocateCommandBuffers(context->getDevice(), &alloc_info, vk_command_buffers));

        InitCommandBuffer(
            GetCommandBuffer(DataTransferCompute), m_command_pool, vk_command_buffers[DataTransferCompute]);
        InitCommandBuffer(GetCommandBuffer(Graphics), m_command_pool, vk_command_buffers[Graphics]);

        m_semaphore.Init(context);

        m_subid.reset();
        m_valid = true;
    }

    void CommandBufferManager::UnInit()
    {
        GetCommandBuffer(DataTransferCompute).UnInit();
        GetCommandBuffer(Graphics).UnInit();

        const auto& device = m_context->getDevice();

        if (m_command_pool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device, m_command_pool, nullptr);
        }
        m_command_pool = VK_NULL_HANDLE;
    }

    void CommandBufferManager::InitCommandBuffer(CommandBuffer&  command_buffer,
                                           VkCommandPool   pool,
                                           VkCommandBuffer vk_command_buffer)
    {
        command_buffer.Initialize(m_context, pool, vk_command_buffer);
        command_buffer.BeginRecording();
    }

    void CommandBufferManager::IssuePipelineBarrier(const VkPipelineStageFlags        src_stages,
                                                    const VkPipelineStageFlags        dst_stages,
                                                    std::vector<VkImageMemoryBarrier> image_memory_barriers)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(DataTransferCompute);
        vkCmdPipelineBarrier(command_buffer.Handle(),
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

    void CommandBufferManager::ClearColorAttachment(VkImage                              vk_image,
                                              VkImageLayout                        vk_image_layout,
                                              const VkClearColorValue&             vk_clear_color,
                                              std::vector<VkImageSubresourceRange> ranges)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(DataTransferCompute);
        vkCmdClearColorImage(command_buffer.Handle(),
                             vk_image,
                             vk_image_layout,
                             &vk_clear_color,
                             ranges.size(),
                             ranges.data());
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::ClearDepthStencil(VkImage                              vk_image,
                                           VkImageLayout                        vk_image_layout,
                                           const VkClearDepthStencilValue&      vk_clear_depth_stencil,
                                           std::vector<VkImageSubresourceRange> ranges)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(DataTransferCompute);
        vkCmdClearDepthStencilImage(command_buffer.Handle(),
                                    vk_image,
                                    vk_image_layout,
                                    &vk_clear_depth_stencil,
                                    ranges.size(),
                                    ranges.data());
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::SubmitCommandBuffers(const std::vector<CommandBuffer*>& command_buffers)
    {
        VkSemaphore              timeline_handle = m_semaphore.GetHandle();
        TimelineSemaphore::Value wait_value      = m_semaphore.GetValue();
        m_last_signal_value                      = m_semaphore.IncreaseValue();

        VkCommandBuffer submitted_buffers[2];
        int             num_command_buffers = 0;

        for (CommandBuffer* command_buffer : command_buffers)
        {
            command_buffer->EndRecording();
            submitted_buffers[num_command_buffers++] = command_buffer->Handle();
        }

        VkTimelineSemaphoreSubmitInfo timeline_info = {};
        timeline_info.sType                     = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timeline_info.pNext                     = nullptr;
        timeline_info.waitSemaphoreValueCount   = 1;
        timeline_info.pWaitSemaphoreValues      = wait_value;
        timeline_info.signalSemaphoreValueCount = 1;
        timeline_info.pSignalSemaphoreValues    = m_last_signal_value;

        VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkSubmitInfo         submit_info = {};
        submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount   = num_command_buffers;
        submit_info.pCommandBuffers      = submitted_buffers;
        submit_info.pNext                = &timeline_info;
        submit_info.waitSemaphoreCount   = 1;
        submit_info.pWaitSemaphores      = &timeline_handle;
        submit_info.pWaitDstStageMask    = &wait_stages;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores    = &timeline_handle;

        VULKAN_API_CALL(vkQueueSubmit(m_context->getGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE));
        VULKAN_API_CALL(vkQueueWaitIdle(m_context->getGraphicsQueue()));
        Wait();

        for (CommandBuffer* command_buffer : command_buffers)
        {
            command_buffer->CommandsSubmitted();
            command_buffer->BeginRecording();
        }
    }

    void CommandBufferManager::EnsureNoDrawCommands()
    {
        if (GetCommandBuffer(Graphics).HasRecordedCommands())
        {
            Submit();
        }
    }

    void CommandBufferManager::BeginRenderPass(Framebuffer& framebuffer)
    { 
        m_active_framebuffer = &framebuffer;
        // EnsureActiveFramebuffer();

        assert(m_active_framebuffer);

        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass            = m_active_framebuffer->render_pass;
        render_pass_info.framebuffer           = m_active_framebuffer->handle;
        render_pass_info.renderArea            = m_active_framebuffer->render_area;
        
        std::vector<VkClearValue> clear_values = {};
        for (auto& attachment : framebuffer.attachments)
        {
            auto clear_value = attachment->GetClearValue();
            if (clear_value.has_value())
            {
                clear_values.push_back(clear_value.value());
            }
        }
        assert(clear_values.size() == framebuffer.attachments.size());

        render_pass_info.clearValueCount = clear_values.size();
        render_pass_info.pClearValues    = clear_values.data();

        auto& graphics_command_buffer = GetCommandBuffer(Graphics);
        vkCmdBeginRenderPass(graphics_command_buffer.Handle(), &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandBufferManager::EndRenderPass(Framebuffer& framebuffer) 
    { 
        // EnsureNoActiveFramebuffer();

        assert(m_active_framebuffer);
        if (m_active_framebuffer)
        {
            auto& graphics_command_buffer = GetCommandBuffer(Graphics);
            vkCmdEndRenderPass(graphics_command_buffer.Handle());
            graphics_command_buffer.CommandRecorded();
            m_active_framebuffer = nullptr;
        }
    }

    void CommandBufferManager::Wait()
    {
        m_semaphore.Wait(m_last_signal_value);
        m_subid.next();
    }

    void CommandBufferManager::Submit()
    {
        const auto& device                = m_context->getDevice();
        auto&       data_transfer_compute = GetCommandBuffer(DataTransferCompute);
        auto&       graphics              = GetCommandBuffer(Graphics);

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
            Framebuffer* active_framebuffer = m_active_framebuffer;
            // EndRenderPass(*active_framebuffer);
            command_buffers[command_buffer_index++] = &graphics;
            SubmitCommandBuffers(std::vector<CommandBuffer*>(command_buffers, command_buffers + command_buffer_index));
            // BeginRenderPass(*active_framebuffer);
        }
        else if (has_data_transfer_compute_work)
        {
            SubmitCommandBuffers(std::vector<CommandBuffer*>(command_buffers, command_buffers + command_buffer_index));
        }
        else
        {
            assert(0);
        }
    }

    void CommandBufferManager::Copy(Texture* src_texture, Buffer* dst_buffer, const std::vector<VkBufferImageCopy>& regions)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(DataTransferCompute);
        vkCmdCopyImageToBuffer(command_buffer.Handle(),
                               src_texture->getImage(),
                               src_texture->getLayout(),
                               dst_buffer->getBuffer(),
                               regions.size(),
                               regions.data());
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::Copy(Buffer* src_buffer, Texture* dst_texture, const std::vector<VkBufferImageCopy>& regions)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(DataTransferCompute);
        vkCmdCopyBufferToImage(command_buffer.Handle(),
                               src_buffer->getBuffer(),
                               dst_texture->getImage(),
                               dst_texture->getLayout(),
                               regions.size(),
                               regions.data());
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::Copy(Buffer* src_buffer, Buffer* dst_buffer, const std::vector<VkBufferCopy>& regions)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(DataTransferCompute);
        vkCmdCopyBuffer(command_buffer.Handle(),
                        src_buffer->getBuffer(),
                        dst_buffer->getBuffer(),
                        regions.size(),
                        regions.data());
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::Blit(Texture* src_tex, Texture* dst_tex, const std::vector<VkImageBlit>& regions)
    {
        Blit(src_tex->getImage(),
                  src_tex->getLayout(),
                  dst_tex->getImage(),
                  dst_tex->getLayout(),
                  regions);
    }

    void CommandBufferManager::Blit(VkImage                         src_image,
                                    VkImageLayout                   src_layout,
                                    VkImage                         dst_image,
                                    VkImageLayout                   dst_layout,
                                    const std::vector<VkImageBlit>& regions)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(DataTransferCompute);
        vkCmdBlitImage(command_buffer.Handle(),
                       src_image,
                       src_layout,
                       dst_image,
                       dst_layout,
                       regions.size(),
                       regions.data(),
                       VK_FILTER_NEAREST);
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::Bind(VkBufferUsageFlags        usage,
                                    std::vector<VkBuffer>     buffers,
                                    std::vector<VkDeviceSize> offsets,
                                    uint32_t                  binding_point)
    {
        assert(!buffers.empty());
        assert(buffers.size() == offsets.size());

        if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
        {
            auto& command_buffer = GetCommandBuffer(Graphics);
            vkCmdBindVertexBuffers(
                command_buffer.Handle(), binding_point, buffers.size(), buffers.data(), offsets.data());
        }
        else if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
        {
            assert(buffers.size() == 1);
            auto& command_buffer = GetCommandBuffer(Graphics);
            vkCmdBindIndexBuffer(command_buffer.Handle(), buffers.front(), offsets.front(), VK_INDEX_TYPE_UINT32);
        }
        else
        {
            assert(0);
        }
    }

    void CommandBufferManager::BindDescriptorSet(const VkDescriptorSet  descriptor_set,
                                           const VkPipelineLayout pipeline_layout,
                                           VkPipelineBindPoint    pipeline_bind_point)
    {
        Type command_type;
        if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)
        {
            EnsureNoDrawCommands();
            command_type = DataTransferCompute;
        }
        if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)
        {
            command_type = Graphics;
        }

        auto& command_buffer = GetCommandBuffer(command_type);
        vkCmdBindDescriptorSets(
            command_buffer.Handle(), pipeline_bind_point, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::BindPipeline(const VkPipeline& pipeline, VkPipelineBindPoint bind_point) {
        Type command_type;
        if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)
        {
            EnsureNoDrawCommands();
            command_type = DataTransferCompute;
        }
        else if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)
        {
            command_type = Graphics;
        }
        else
        {
            assert(0);
        }

        auto& command_buffer = GetCommandBuffer(command_type);
        vkCmdBindPipeline(command_buffer.Handle(), bind_point, pipeline);
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::PipelineBarrier(const VkPipelineStageFlags        src_stage,
                                               const VkPipelineStageFlags        dst_stage,
                                               std::vector<VkImageMemoryBarrier> barriers)
    {
        auto& command_buffer = GetCommandBuffer(DataTransferCompute);
        vkCmdPipelineBarrier(command_buffer.Handle(),
                             src_stage,
                             dst_stage,
                             VK_DEPENDENCY_BY_REGION_BIT,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             barriers.size(),
                             barriers.data());
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::Dispatch(int groups_x_len, int groups_y_len, int groups_z_len)
    {
        EnsureNoDrawCommands();

        CommandBuffer& command_buffer = GetCommandBuffer(DataTransferCompute);
        vkCmdDispatch(command_buffer.Handle(), groups_x_len, groups_y_len, groups_z_len);
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::Dispatch(Buffer* storage_buffer)
    {
        EnsureNoDrawCommands();

        CommandBuffer& command_buffer = GetCommandBuffer(DataTransferCompute);
        vkCmdDispatchIndirect(command_buffer.Handle(), storage_buffer->getBuffer(), 0);
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::SetViewport(VkViewport viewport)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(Graphics);
        vkCmdSetViewport(command_buffer.Handle(), 0, 1, &viewport);
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::SetScissor(VkRect2D scissor)
    {
        CommandBuffer& command_buffer = GetCommandBuffer(Graphics);
        vkCmdSetScissor(command_buffer.Handle(), 0, 1, &scissor);
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::Draw(Mesh* mesh)
    { 
        this->Draw(mesh->getVertexCount(), 1);
    }

    void CommandBufferManager::Draw(uint32_t vertex_count, uint32_t instance_count)
    {
        // EnsureActiveFramebuffer();

        CommandBuffer& command_buffer = GetCommandBuffer(Graphics);
        vkCmdDraw(command_buffer.Handle(), vertex_count, instance_count, 0, 0);
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::DrawIndexed(Mesh* mesh) {
        // EnsureActiveFramebuffer();

        CommandBuffer& command_buffer = GetCommandBuffer(Graphics);
        vkCmdDrawIndexed(command_buffer.Handle(), mesh->getIndexCount(), 1, 0, 0, 0);
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::NextSubpass()
    {
        CommandBuffer& command_buffer = GetCommandBuffer(Graphics);
        vkCmdNextSubpass(command_buffer.Handle(), VK_SUBPASS_CONTENTS_INLINE);
        command_buffer.CommandRecorded();
    }

    void CommandBufferManager::ActivateFramebuffer(Framebuffer& framebuffer) { 
        // if (m_active_framebuffer)
        // {
        //     EndRenderPass(*m_active_framebuffer);
        // }

        // assert(m_active_framebuffer == nullptr);
        m_active_framebuffer = &framebuffer;

        // BeginRenderPass(framebuffer);
    }

    CommandBuffer& CommandBufferManager::GetCommandBuffer(Type type) { return m_buffers[type]; }

    SubmissionID& CommandBufferManager::GetSubmissionId() { return m_subid; }

    void CommandBufferManager::RenderImGui() {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), GetCommandBuffer(Graphics).Handle());
    }

} // namespace Chandelier