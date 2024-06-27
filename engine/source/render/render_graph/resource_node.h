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


    };

    class BufferNode: public ResourceNode
    {
    public:
        friend class RenderGraph;

        BufferNode();



    };


}