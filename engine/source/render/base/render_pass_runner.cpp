#include "render_pass_runner.h"

#include "Texture.h"
#include "resource/asset_manager/asset_manager.h"

namespace Chandelier
{
    RenderPassRunner::~RenderPassRunner() { UnInit(); }

    void RenderPassRunner::Initialize(std::shared_ptr<RenderPass> render_pass)
    {
        UnInit();
        m_render_pass = render_pass;
    }

    void RenderPassRunner::UnInit() { m_render_pass = nullptr; }

    void RenderPassRunner::Run() { 
        m_render_pass->Draw();
    }

    void RenderPassRunner::Save(std::string_view path, uint32_t framebuffer_index, uint32_t attachment_index)
    { 
        auto attachment = m_render_pass->GetAttachment(framebuffer_index, framebuffer_index);
        attachment->TransferLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        SaveTexture(attachment, path, framebuffer_index, attachment_index);
    }


} // namespace Chandelier