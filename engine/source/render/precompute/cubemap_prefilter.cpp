//#include "brdf_lut.h"
//
//namespace Chandelier
//{
//    void BRDFLutPass::Initialize(std::shared_ptr<BaseRenderPassInitInfo> info)
//    {
//        m_pass_info = std::dynamic_pointer_cast<BRDFLutInitInfo>(info);
//    }
//
//    void BRDFLutPass::UnInit() {}
//
//    void BRDFLutPass::Recreate() {}
//
//    const VkRenderPass* BRDFLutPass::GetRenderPass() {}
//
//    void BRDFLutPass::SetupAttachments() {}
//    void BRDFLutPass::ResetAttachments() {}
//
//    void BRDFLutPass::SetupPipeline() {}
//    void BRDFLutPass::ResetPipeline() {}
//
//    void BRDFLutPass::SetupFramebuffers() {}
//    void BRDFLutPass::ResetFramebuffers() {}
//
//    void BRDFLutPass::PreDrawSetup() {}
//
//    void BRDFLutPass::Draw() {}
//
//    void BRDFLutPass::PostDrawCallback() {}
//} // namespace Chandelier