#pragma once

#include "VkCommon.h"

namespace Chandelier
{
    // class Framebuffer
    // {
    //     int32_t       m_width;
    //     int32_t       m_height;
    //     VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    //     VkRenderPass  m_render_pass = VK_NULL_HANDLE;
    // 
    //     bool m_enable_srgb;
    // 
    //     std::vector<std::shared_ptr<Texture>> m_attachment_views;
    // 
    // public:
    //     Framebuffer() = default;
    //     ~Framebuffer();
    // 
    //     VkRenderPass  GetRenderPass() const { return m_render_pass; }
    //     VkFramebuffer GetFramebuffer() const { return m_framebuffer; };
    //     int32_t       GetWidth() const { return m_width; }
    //     int32_t       GetHeight() const { return m_height; }
    // 
    //     std::vector<std::shared_ptr<Texture>> GetAttachmentViews() const
    //     {
    //         return m_attachment_views;
    //     }
    // 
    //     void SetAttachmentSize(int32_t size)
    //     {
    //         m_attachment_views.clear();
    //         m_attachment_views.resize(size);
    //     }
    // 
    //     void CreateRenderPass();
    // };

    struct Framebuffer
    {
        VkRect2D                              render_area;
        VkFramebuffer                         handle      = VK_NULL_HANDLE;
        VkRenderPass                          render_pass = VK_NULL_HANDLE;
        std::vector<std::shared_ptr<Texture>> attachments;
    };

} // namespace Chandelier