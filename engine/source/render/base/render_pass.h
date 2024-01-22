#pragma once

#include <memory>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "render/base/render_pass_base.h"

namespace Chandelier
{
    class GlobalRenderResource;
    class Descriptor;
    class Image;

    struct RenderPassContext
    {
        std::shared_ptr<VKContext> vk_context;
    };

    struct BaseRenderPassInitInfo
    {
        virtual ~BaseRenderPassInitInfo() = default;
        RenderPassContext render_context;
    };

    class RenderPass : public RenderPassBase
    {
    public:
        // struct FrameBufferAttachment
        // {
        //     VkImage        image;
        //     VkDeviceMemory mem;
        //     VkImageView    view;
        //     VkFormat       format;
        // };

        struct Framebuffer
        {
            int           width;
            int           height;
            VkFramebuffer framebuffer;
            VkRenderPass  render_pass;

            std::vector<std::shared_ptr<Image>> attachments;
        };

        // struct Descriptor
        // {
        //     VkDescriptorSetLayout layout;
        //     VkDescriptorSet       descriptor_set;
        // };

        struct RenderPipelineBase
        {
            VkPipelineLayout layout;
            VkPipeline       pipeline;
        };

        VkRenderPass             getRenderPass() const;
        std::vector<VkImageView> getFramebufferImageViews() const;

        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) = 0;

        std::shared_ptr<GlobalRenderResource> m_global_render_resource;

        std::vector<Descriptor>         m_descriptor_infos;
        std::vector<RenderPipelineBase> m_render_pipelines;
        Framebuffer                     m_framebuffer;

        RenderPassContext m_context;
    };

} // namespace Chandelier
