//#include "render_graph.h"
//
//namespace Chandelier
//{
//    RenderGraph::RenderGraphBuilder* RenderGraph::RenderGraphBuilder::SetContext(std::shared_ptr<VKContext> ctx) {
//        m_ctx = ctx;
//        return this;
//    }
//
//    RenderGraph::RenderGraphBuilder* RenderGraph::RenderGraphBuilder::EnableMemoryAliasing() {
//        m_use_memory_aliasing = true; 
//        return this;
//    }
//
//    std::shared_ptr<RenderGraph> RenderGraph::Create(const RenderGraphSetupFunction& setup) 
//    {
//        RenderGraphBuilder builder = {};
//        std::shared_ptr<RenderGraph> graph;
//        setup(builder);
//        
//        return graph;
//    }
//}