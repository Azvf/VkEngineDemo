#pragma once

#include "render_graph.h"

namespace Chandelier
{
    class VKContext;
    class RenderGraphBackend : public RenderGraph
    {
        friend class RenderGraph;    
    public:
        RenderGraphBackend(const RenderGraphBuilder& builder);
        ~RenderGraphBackend() = default;

        bool                       compile();
        virtual uint64_t           execute(RenderGraphProfiler* profiler = nullptr);
        virtual std::shared_ptr<VKContext> GetBackendContext();
        
        virtual uint32_t
                         collect_garbage(uint64_t critical_frame,
                                         uint32_t tex_with_tags = Render_Graph_Default_Resource_Tag | Render_Graph_Dynamic_Resource_Tag,
                                         uint32_t tex_without_tags = 0,
                                         uint32_t buf_with_tags = Render_Graph_Default_Resource_Tag | Render_Graph_Dynamic_Resource_Tag,
                                         uint32_t buf_without_tags = 0)  ;
        virtual uint32_t collect_texture_garbage(uint64_t critical_frame,
                                                 uint32_t with_tags = Render_Graph_Default_Resource_Tag |
                                                                      Render_Graph_Dynamic_Resource_Tag,
                                                 uint32_t without_tags = 0)  ;
        virtual uint32_t collect_buffer_garbage(uint64_t critical_frame,
                                                uint32_t with_tags = Render_Graph_Default_Resource_Tag |
                                                                     Render_Graph_Dynamic_Resource_Tag,
                                                uint32_t without_tags = 0)  ;

    };


}