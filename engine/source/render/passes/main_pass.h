#pragma once

#include "base/render_pass.h"

namespace Chandelier
{
    enum AntiAliasing : uint8_t
    {
        None_AA     = 0,
        Enable_MSAA = 1,
        Enable_FXAA = 2,
    };

    struct MainRenderPassInitInfo : public BaseRenderPassInitInfo
    {
        virtual ~MainRenderPassInitInfo() = default;
        AntiAliasing aa;
    };

    class MainRenderPass : public RenderPass
    {
        enum eAttachment : uint8_t
        {
            Color_Attachment        = 0,
            DepthStencil_Attachment = 1,
            Attachment_Max_Count
        };

    public:
        MainRenderPass() = default;
        ~MainRenderPass();

    public:
        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) override;

    private:
        std::shared_ptr<MainRenderPassInitInfo> m_pass_info;
    };

} // namespace Chandelier