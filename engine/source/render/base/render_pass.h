#pragma once

#include "render/base/render_pass_base.h"
#include "resource/asset_manager/asset_manager.h"

#include "VkCommon.h"
#include "Framebuffer.h"

namespace Chandelier
{
    class GlobalRenderResource;
    class DescriptorTracker;
    class MainPassUniformBuffer;
    class RenderResources;

    enum eAttachment : uint8_t
    {
        Shadowmap_Attachment    = 0,
        Color_Attachment        = 1,
        DepthStencil_Attachment = 2,
        Attachment_Max_Count,
        Resolve_Attachment = Attachment_Max_Count
    };

    /**
     * @todo: just listed the main pass and the dependent subpasses, 
     * do the render pass management later
     */
    enum eRenderPass : uint8_t
    {
        Shadowmap_Pass = 0,
        Main_Pass      = 1,
        Skybox_Pass    = 2,
        UI_Pass        = 3,
        Render_Pass_Count,
    };
    
    enum eAntiAliasing : uint8_t
    {
        None_AA     = 0,
        Enable_MSAA = 1,
        Enable_FXAA = 2,
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
        std::shared_ptr<RenderResources> render_resources;
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

        virtual void Recreate() = 0;

        virtual const VkRenderPass* GetRenderPass() = 0;

        virtual void PreDrawSetup() = 0;
        virtual void Draw() = 0;
        virtual void PostDrawCallback() = 0;

        virtual std::shared_ptr<Texture> GetAttachment(uint32_t framebuffer_index, uint32_t attachment_index)
        {
            assert(m_framebuffers.size());
            if (!m_framebuffers.size())
                return nullptr;
            return m_framebuffers.at(framebuffer_index).attachments.at(attachment_index);
        }

        std::vector<Framebuffer> m_framebuffers;
    };

} // namespace Chandelier
