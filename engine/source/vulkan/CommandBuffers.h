#pragma once

#include "runtime/core/base/base_utility.h"

#include "CommandBuffer.h"
#include "ResourceTracker.h"
#include "TimelineSemaphore.h"
#include "Framebuffer.h"
#include "Descriptor.h"

namespace Chandelier
{
    class VKContext;
    class Texture;
    class Buffer;
    class Mesh;

    class CommandBufferManager : public NonCopyable
    {
        enum Type : uint8_t
        {
            DataTransferCompute    = 0,
            Graphics               = 1,
            CommandBufferTypeCount = 2,
        };

    public:
        CommandBufferManager() = default;
        virtual ~CommandBufferManager();

        bool Valid();
        void Initialize(std::shared_ptr<VKContext> context);
        void UnInit();

        SubmissionID&  GetSubmissionId();

        void RenderImGui();

        void IssuePipelineBarrier(const VkPipelineStageFlags        src_stages,
                                  const VkPipelineStageFlags        dst_stages,
                                  std::vector<VkImageMemoryBarrier> image_memory_barriers);

        void ClearColorAttachment(VkImage                              vk_image,
                                  VkImageLayout                        vk_image_layout,
                                  const VkClearColorValue&             vk_clear_color,
                                  std::vector<VkImageSubresourceRange> ranges);

        void ClearDepthStencil(VkImage                              vk_image,
                               VkImageLayout                        vk_clear_depth_stencil,
                               const VkClearDepthStencilValue&      vk_clear_color,
                               std::vector<VkImageSubresourceRange> ranges);

        void Copy(Texture* src_texture, Buffer* dst_buffer, const std::vector<VkBufferImageCopy>& regions);
        void Copy(Buffer* src_buffer, Texture* dst_texture, const std::vector<VkBufferImageCopy>& regions);
        void Copy(Buffer* src_buffer, Buffer* dst_buffer, const std::vector<VkBufferCopy>& regions);
        void Copy(Texture* src_texture, Texture* dst_texture, const std::vector<VkImageCopy>& regions);

        void Blit(VkImage                         src_image,
                  VkImageLayout                   src_layout,
                  VkImage                         dst_image,
                  VkImageLayout                   dst_layout,
                  const std::vector<VkImageBlit>& regions);
        void Blit(Texture* src_tex, Texture* dst_tex, const std::vector<VkImageBlit>& regions);

        void Bind(VkBufferUsageFlags        usage,
                  std::vector<VkBuffer>     buffers,
                  std::vector<VkDeviceSize> offsets,
                  uint32_t                  binding_point = 0);
        void BindDescriptorSet(const VkDescriptorSet  descriptor_set,
                               const VkPipelineLayout pipeline_layout,
                               VkPipelineBindPoint    pipeline_bind_point);
        void BindPipeline(const VkPipeline& pipeline, VkPipelineBindPoint bind_point);
        void PipelineBarrier(const VkPipelineStageFlags src_stage,
                             const VkPipelineStageFlags dst_stage,
                             std::vector<VkImageMemoryBarrier> barriers);
        void SetViewport(VkViewport viewport);
        void SetScissor(VkRect2D scissor);

        // todo: push constants

        void Dispatch(int groups_x_len, int groups_y_len, int groups_z_len);
        void Dispatch(Buffer* storage_buffer);

        // todo: modify the interface to take a pointer type to avoid a de-activate api
        void ActivateFramebuffer(Framebuffer& framebuffer);

        void Draw(Mesh* mesh);
        void Draw(uint32_t vertex_count, uint32_t instance_count);
        
        // todo: optimize interface
        void DrawIndexed(Mesh* mesh);

        void NextSubpass();

        void BeginRenderPass(Framebuffer& framebuffer);
        void EndRenderPass(Framebuffer& framebuffer);

        void Submit();
        void Wait();

    private:
        CommandBuffer& GetCommandBuffer(Type type);

        void InitCommandBuffer(CommandBuffer& command_buffer, VkCommandPool pool, VkCommandBuffer vk_command_buffer);
        void SubmitCommandBuffers(const std::vector<CommandBuffer*>& command_buffers);

        void EnsureNoDrawCommands();

    private:
        std::shared_ptr<VKContext> m_context;
        VkCommandPool              m_command_pool                    = VK_NULL_HANDLE;
        CommandBuffer              m_buffers[CommandBufferTypeCount] = {};
        bool                       m_valid                           = false;
        TimelineSemaphore          m_semaphore;

        /**
         * Last submitted timeline value, what can be used to validate that all commands related
         * submitted by this command buffers have been finished.
         */
        TimelineSemaphore::Value m_last_signal_value;

        SubmissionID m_subid;

        Framebuffer* m_active_framebuffer = nullptr;
    };

} // namespace Chandelier