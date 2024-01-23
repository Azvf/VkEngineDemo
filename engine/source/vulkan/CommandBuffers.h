#pragma once

#include "CommandBuffer.h"
#include "VkCreateInfo.h"

namespace Chandelier
{
    class VKContext;
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

        bool           Valid();
        void           Initialize(std::shared_ptr<VKContext> context);
        CommandBuffer& GetCommandBuffer(Type type);

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

    private:
        void InitCommandBuffer(CommandBuffer&  command_buffer,
                               VkCommandPool   pool,
                               VkCommandBuffer vk_command_buffer);
        void SubmitCommandBuffers(std::shared_ptr<VKContext> context, const std::vector<CommandBuffer*>& command_buffers);

    private:
        std::shared_ptr<VKContext> m_context;
        VkCommandPool              m_command_pool       = VK_NULL_HANDLE;
        CommandBuffer              m_buffers[Type::Max] = {};
        bool                       m_valid              = false;
    };

} // namespace Chandelier