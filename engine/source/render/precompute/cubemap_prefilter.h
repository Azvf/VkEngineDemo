//#pragma once
//
//#include "base/render_pass.h"
//
//namespace Chandelier
//{
//    struct BRDFLutInitInfo : public BaseRenderPassInitInfo
//    {
//        virtual ~BRDFLutInitInfo() = default;
//    };
//
//    class BRDFLutPass : public RenderPass
//    {
//    public:
//        BRDFLutPass() = default;
//        virtual ~BRDFLutPass();
//
//    public:
//        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) override;
//        virtual void UnInit() override;
//
//        virtual void Recreate() override;
//
//        virtual const VkRenderPass* GetRenderPass() override;
//
//        virtual void PreDrawSetup() override;
//        virtual void Draw() override;
//        virtual void PostDrawCallback() override;
//
//    private:
//        void SetupAttachments();
//        void ResetAttachments();
//
//        void SetupPipeline();
//        void ResetPipeline();
//
//        void SetupFramebuffers();
//        void ResetFramebuffers();
//
//    private:
//        std::shared_ptr<BRDFLutInitInfo> m_pass_info;
//    };
//
//} // namespace Chandelier