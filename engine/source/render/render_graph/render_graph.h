#pragma once

#include <functional>
#include <memory>

#include "render_graph_base.h"

namespace Chandelier
{
    class VKContext;

    class RenderGraph
    {
    public:
        class RenderGraphBuilder
        {
        public:
            friend class RenderGraph;

            RenderGraphBuilder* SetContext(std::shared_ptr<VKContext> ctx);
            RenderGraphBuilder* EnableMemoryAliasing();

        private:
            bool                       m_use_memory_aliasing = false;
            std::shared_ptr<VKContext> m_ctx;
        };

        using RenderGraphSetupFunction = std::function<void(class RenderGraph::RenderGraphBuilder&)>;
        static std::shared_ptr<RenderGraph> Create(const RenderGraphSetupFunction& setup);
        // static void         Destroy(RenderGraph* g);

            class SKR_RENDER_GRAPH_API RenderPassBuilder
        {
        public:
            friend class RenderGraph;
            RenderPassBuilder& set_name(const char8_t* name) SKR_NOEXCEPT;
            // textures
            RenderPassBuilder& read(uint32_t set, uint32_t binding, TextureSRVHandle handle) SKR_NOEXCEPT;
            RenderPassBuilder& read(const char8_t* name, TextureSRVHandle handle) SKR_NOEXCEPT;
            RenderPassBuilder& write(uint32_t         mrt_index,
                                     TextureRTVHandle handle,
                                     ECGPULoadAction  load_action  = CGPU_LOAD_ACTION_CLEAR,
                                     CGPUClearValue   clear_color  = fastclear_0000,
                                     ECGPUStoreAction store_action = CGPU_STORE_ACTION_STORE) SKR_NOEXCEPT;
            RenderPassBuilder& resolve_msaa(uint32_t mrt_index, TextureSubresourceHandle handle);

            RenderPassBuilder& set_depth_stencil(TextureDSVHandle handle,
                                                 ECGPULoadAction  dload_action  = CGPU_LOAD_ACTION_CLEAR,
                                                 ECGPUStoreAction dstore_action = CGPU_STORE_ACTION_STORE,
                                                 ECGPULoadAction  sload_action  = CGPU_LOAD_ACTION_CLEAR,
                                                 ECGPUStoreAction sstore_action = CGPU_STORE_ACTION_STORE) SKR_NOEXCEPT;

            // buffers
            RenderPassBuilder& read(const char8_t* name, BufferRangeHandle handle) SKR_NOEXCEPT;
            RenderPassBuilder& read(uint32_t set, uint32_t binding, BufferRangeHandle handle) SKR_NOEXCEPT;
            RenderPassBuilder& write(uint32_t set, uint32_t binding, BufferHandle handle) SKR_NOEXCEPT;
            RenderPassBuilder& write(const char8_t* name, BufferHandle handle) SKR_NOEXCEPT;
            RenderPassBuilder& use_buffer(PipelineBufferHandle buffer, ECGPUResourceState requested_state) SKR_NOEXCEPT;

            RenderPassBuilder& set_pipeline(CGPURenderPipelineId pipeline) SKR_NOEXCEPT;
            RenderPassBuilder& set_root_signature(CGPURootSignatureId signature) SKR_NOEXCEPT;

        protected:
            RenderPassBuilder(RenderGraph& graph, RenderPassNode& pass) SKR_NOEXCEPT;
            RenderGraph&    graph;
            RenderPassNode& node;
        };




    };
} // namespace Chandelier
