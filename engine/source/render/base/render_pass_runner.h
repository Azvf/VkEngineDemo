#pragma once

#include "render_pass.h"

namespace Chandelier
{
    class RenderPassRunner
    {
    public:
        RenderPassRunner() = default;
        ~RenderPassRunner();

        void Initialize(std::shared_ptr<RenderPass> render_pass);
        void UnInit();

        void Run();

        void Save(std::string_view path, uint32_t framebuffer_index = 0, uint32_t attachment_index = 0);

    private:
        std::shared_ptr<RenderPass> m_render_pass;
    };

} // namespace Chandelier