#include "main_pass.h"

#include "Texture.h"
#include "VkCreateInfo.h"

namespace Chandelier
{
    MainRenderPass::~MainRenderPass() {}

    void MainRenderPass::Initialize(std::shared_ptr<BaseRenderPassInitInfo> info)
    {
        std::shared_ptr<VKContext> context = info->render_context.vk_context;
        m_pass_info = std::dynamic_pointer_cast<MainRenderPassInitInfo>(info);
        m_framebuffer.attachments.resize(Attachment_Max_Count);

        auto& color_attachment         = m_framebuffer.attachments[Color_Attachment];
        auto& depth_stencil_attachment = m_framebuffer.attachments[DepthStencil_Attachment];

        color_attachment.InitTex2D(context,
                                   m_framebuffer.width,
                                   m_framebuffer.height,
                                   1,
                                   1,
                                   VK_FORMAT_R16G16B16A16_SFLOAT,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        depth_stencil_attachment.InitTex2D(context,
                                           m_framebuffer.width,
                                           m_framebuffer.height,
                                           1,
                                           1,
                                           VK_FORMAT_D32_SFLOAT_S8_UINT,
                                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                               VK_IMAGE_USAGE_SAMPLED_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
        
    }

} // namespace Chandelier