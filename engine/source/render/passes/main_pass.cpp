#include "main_pass.h"

#include "Image.h"

namespace Chandelier
{
    enum Attachment : uint8_t
    {
        Color_Attachment = 0,
        Depth_Attachment = 1,
        Attachment_Max_Count
    };

    MainRenderPass::~MainRenderPass() {}

    void MainRenderPass::Initialize(std::shared_ptr<BaseRenderPassInitInfo> info)
    {
        m_pass_info = std::dynamic_pointer_cast<MainRenderPassInitInfo>(info);
        m_framebuffer.attachments.resize(Attachment_Max_Count);
        
        ImageCreateInfo image_create_info {};
        image_create_info.context = m_pass_info->render_context.vk_context; 
        m_framebuffer.attachments[Color_Attachment] = Image::Create();


                

    }

} // namespace Chandelier