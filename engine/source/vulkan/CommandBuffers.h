#pragma once

#include "CommandBuffer.h"
#include "TimelineSemaphore.h"
#include "ResourceTracker.h"
#include "VkCreateInfo.h"

namespace Chandelier
{
    class VKContext;
    class Texture;
    class Buffer;

    class CommandBuffers
    {
        enum Type : uint8_t
        {
            DataTransferCompute = 0,
            Graphics            = 1,
            Max                 = 2,
        };

    public:
        CommandBuffers() = default;
        ~CommandBuffers();

        bool Valid();
        void Initialize(std::shared_ptr<VKContext> context);
        void Free();

        CommandBuffer& GetCommandBuffer(Type type);
        SubmissionID&  GetSubmissionId();

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

        void Copy(std::shared_ptr<Buffer>  src_buffer,
                  std::shared_ptr<Texture> dst_texture,
                  const std::vector<VkBufferImageCopy>& regions);



        void Submit();
        void Wait();

    private:
        void InitCommandBuffer(CommandBuffer&  command_buffer,
                               VkCommandPool   pool,
                               VkCommandBuffer vk_command_buffer);
        void SubmitCommandBuffers(const std::vector<CommandBuffer*>& command_buffers);
        void EnsureNoDrawCommands();

    private:
        std::shared_ptr<VKContext> m_context;
        VkCommandPool              m_command_pool       = VK_NULL_HANDLE;
        CommandBuffer              m_buffers[Type::Max] = {};
        bool                       m_valid              = false;
        TimelineSemaphore          m_semaphore;

        /**
         * Last submitted timeline value, what can be used to validate that all commands related
         * submitted by this command buffers have been finished.
         */
        TimelineSemaphore::Value m_last_signal_value;

        SubmissionID m_subid;
    };

} // namespace Chandelier