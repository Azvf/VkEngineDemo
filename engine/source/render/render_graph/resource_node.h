#pragma once

#include "render_graph_base.h"

namespace Chandelier
{
    class ResourceNode : public RenderGraphNode
    {
    public:
        friend class RenderGraph;

        struct LifeSpan
        {
            uint32_t from;
            uint32_t to;
        };

        ResourceNode(ERenderResourceType type);

        bool Imported() { return m_imported; }
        LifeSpan GetLifeSpan() const;

    protected:
        bool     m_imported       = false;
        LifeSpan m_frame_lifespan = {UINT32_MAX, UINT32_MAX};
    };

    class TextureNode : public ResourceNode
    {
    public:
        friend class RenderGraph;

        TextureNode();

    protected:
        Texture*     m_texture;
        TextureNode* m_frame_aliasing_src;
        bool         m_frame_aliasing;
    };

    class BufferNode: public ResourceNode
    {
    public:
        friend class RenderGraph;

        BufferNode();

        void Reimport();

        BufferHandle Handle();

    protected:
        Buffer* m_buffer;
    };


}