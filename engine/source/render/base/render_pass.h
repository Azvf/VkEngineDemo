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
        UI_Attachment           = 2,
        Attachment_Max_Count
    };

    enum RenderPassSubpass : uint8_t
    {
        Subpass_Base_Pass   = 0,
        Subpass_UI_Pass     = 1,
        Subpass_Count,
    };

    struct RenderPassContext
    {
        std::shared_ptr<VKContext> vk_context;
    };

    struct BaseRenderPassInitInfo
    {
        virtual ~BaseRenderPassInitInfo() = default;
        RenderPassContext render_context;
        uint32_t          width, height;
    };

    class RenderPass : public RenderPassBase
    {
    public:
        struct RenderPipelineBase
        {
            VkPipelineLayout layout      = VK_NULL_HANDLE;
            VkPipeline       pipeline    = VK_NULL_HANDLE;
            VkRenderPass     render_pass = VK_NULL_HANDLE;
        } m_render_pipeline;

        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) = 0;
        virtual void UnInit() = 0;

        virtual const VkRenderPass* GetRenderPass() = 0;

        virtual void PreDrawSetup() = 0;
        virtual void Draw() = 0;
        virtual void PostDrawCallback() = 0;

        std::shared_ptr<GlobalRenderResource> m_global_render_resource;

        // std::vector<Descriptor>         m_descriptor_infos;
        std::vector<Framebuffer> m_framebuffers;
        
        RenderPassContext m_context;
    };

} // namespace Chandelier
