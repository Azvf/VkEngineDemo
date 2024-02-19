#pragma once

#include "render/base/render_pass_base.h"
#include "VkCommon.h"
#include "Framebuffer.h"

namespace Chandelier
{
    class GlobalRenderResource;
    class Descriptor;

    enum eAttachment : uint8_t
    {
        Color_Attachment        = 0,
        DepthStencil_Attachment = 1,
        Attachment_Max_Count
    };

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
        struct RenderPipelineBase
        {
            VkPipelineLayout layout;
            VkPipeline       pipeline;
        };

        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) = 0;

        virtual void PreDrawSetup() = 0;
        virtual void Draw() = 0;
        virtual void PostDrawCallback() = 0;

        std::shared_ptr<GlobalRenderResource> m_global_render_resource;

        // std::vector<Descriptor>         m_descriptor_infos;
        // std::vector<RenderPipelineBase> m_render_pipelines;
        std::vector<Framebuffer> m_framebuffers;
        
        RenderPassContext m_context;
    };

} // namespace Chandelier
