#pragma once

#include <functional>
#include <memory>

#include "render_graph_base.h"

namespace Chandelier
{
    class VKContext;
    class Blackboard;

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

        class  RenderPassBuilder
        {
        public:
            friend class RenderGraph;
            RenderPassBuilder& set_name(std::string_view name);
            // textures
            RenderPassBuilder& read(uint32_t set, uint32_t binding, TextureSRVHandle handle) ;
            RenderPassBuilder& read(std::string_view name, TextureSRVHandle handle);
            RenderPassBuilder& write(uint32_t         mrt_index,
                                     TextureRTVHandle handle,
                                     ECGPULoadAction  load_action  = CGPU_LOAD_ACTION_CLEAR,
                                     CGPUClearValue   clear_color  = fastclear_0000,
                                     ECGPUStoreAction store_action = CGPU_STORE_ACTION_STORE) ;
            RenderPassBuilder& resolve_msaa(uint32_t mrt_index, TextureSubresourceHandle handle);

            RenderPassBuilder& set_depth_stencil(TextureDSVHandle handle,
                                                 ECGPULoadAction  dload_action  = CGPU_LOAD_ACTION_CLEAR,
                                                 ECGPUStoreAction dstore_action = CGPU_STORE_ACTION_STORE,
                                                 ECGPULoadAction  sload_action  = CGPU_LOAD_ACTION_CLEAR,
                                                 ECGPUStoreAction sstore_action = CGPU_STORE_ACTION_STORE) ;

            // buffers
            RenderPassBuilder& read(std::string_view name, BufferRangeHandle handle);
            RenderPassBuilder& read(uint32_t set, uint32_t binding, BufferRangeHandle handle) ;
            RenderPassBuilder& write(uint32_t set, uint32_t binding, BufferHandle handle) ;
            RenderPassBuilder& write(std::string_view name, BufferHandle handle);
            RenderPassBuilder& use_buffer(PipelineBufferHandle buffer, ECGPUResourceState requested_state) ;

            RenderPassBuilder& set_pipeline(CGPURenderPipelineId pipeline) ;
            RenderPassBuilder& set_root_signature(CGPURootSignatureId signature) ;

        protected:
            RenderPassBuilder(RenderGraph& graph, RenderPassNode& pass) ;
            RenderGraph&    graph;
            RenderPassNode& node;
        };

        class ComputePassBuilder
        {
        public:
            friend class RenderGraph;
            ComputePassBuilder& set_name(std::string_view name) ;
            ComputePassBuilder& read(uint32_t set, uint32_t binding, TextureSRVHandle handle) ;
            ComputePassBuilder& read(std::string_view name, TextureSRVHandle handle) ;
            ComputePassBuilder& readwrite(uint32_t set, uint32_t binding, TextureUAVHandle handle) ;
            ComputePassBuilder& readwrite(std::string_view name, TextureUAVHandle handle) ;
            ComputePassBuilder& read(uint32_t set, uint32_t binding, BufferHandle handle) ;
            ComputePassBuilder& read(std::string_view name, BufferHandle handle) ;
            ComputePassBuilder& readwrite(uint32_t set, uint32_t binding, BufferHandle handle) ;
            ComputePassBuilder& readwrite(std::string_view name, BufferHandle handle) ;
            ComputePassBuilder& set_pipeline(CGPUComputePipelineId pipeline) ;
            ComputePassBuilder& set_root_signature(CGPURootSignatureId signature) ;

        protected:
            ComputePassBuilder(RenderGraph& graph, ComputePassNode& pass) ;
            RenderGraph&     graph;
            ComputePassNode& node;
        };


        class  CopyPassBuilder
        {
        public:
            friend class RenderGraph;
            CopyPassBuilder& set_name(std::string_view name) ;
            CopyPassBuilder& can_be_lone() ;
            CopyPassBuilder&
            texture_to_texture(TextureSubresourceHandle src,
                               TextureSubresourceHandle dst,
                               ECGPUResourceState       out_state = CGPU_RESOURCE_STATE_COPY_DEST) ;
            CopyPassBuilder&
            buffer_to_buffer(BufferRangeHandle  src,
                             BufferRangeHandle  dst,
                             ECGPUResourceState out_state = CGPU_RESOURCE_STATE_COPY_DEST) ;
            CopyPassBuilder&
                             buffer_to_texture(BufferRangeHandle        src,
                                               TextureSubresourceHandle dst,
                                               ECGPUResourceState       out_state = CGPU_RESOURCE_STATE_COPY_DEST) ;
            CopyPassBuilder& from_buffer(BufferRangeHandle src) ;

        protected:
            CopyPassBuilder(RenderGraph& graph, CopyPassNode& pass) noexcept;
            RenderGraph&  graph;
            CopyPassNode& node;
        };

        class  PresentPassBuilder
        {
        public:
            friend class RenderGraph;

            PresentPassBuilder& set_name(std::string_view name) ;
            PresentPassBuilder& swapchain(CGPUSwapChainId chain, uint32_t index) ;
            PresentPassBuilder& texture(TextureHandle texture, bool is_backbuffer = true) ;

        protected:
            PresentPassBuilder(RenderGraph& graph, PresentPassNode& present) noexcept;
            RenderGraph&     graph;
            PresentPassNode& node;
        };

        class  BufferBuilder
        {
        public:
            friend class RenderGraph;
            BufferBuilder& set_name(std::string_view name) ;
            BufferBuilder& with_tags(uint32_t tags) ;
            BufferBuilder& import(CGPUBufferId buffer, ECGPUResourceState init_state) ;
            BufferBuilder& allocate_dedicated() ;
            BufferBuilder&
            structured(uint64_t first_element, uint64_t element_count, uint64_t element_stride) ;
            BufferBuilder& size(uint64_t size) ;
            BufferBuilder& with_flags(CGPUBufferCreationFlags flags) ;
            BufferBuilder& memory_usage(ECGPUMemoryUsage mem_usage) ;
            BufferBuilder& allow_shader_readwrite() ;
            BufferBuilder& allow_shader_read() ;
            BufferBuilder& as_upload_buffer() ;
            BufferBuilder& as_vertex_buffer() ;
            BufferBuilder& as_index_buffer() ;
            BufferBuilder& as_uniform_buffer() ;
            BufferBuilder& prefer_on_device() ;
            BufferBuilder& prefer_on_host() ;

        protected:
            BufferBuilder(RenderGraph& graph, BufferNode& node) ;
            RenderGraph& graph;
            BufferNode&  node;
        };

        class TextureBuilder
        {
        public:
            friend class RenderGraph;
            TextureBuilder& set_name(std::string_view name);
            TextureBuilder& with_flags(CGPUTextureCreationFlags tags);
            TextureBuilder& with_tags(uint32_t tags);
            TextureBuilder& import(CGPUTextureId texture, ECGPUResourceState init_state);
            TextureBuilder& extent(uint64_t width, uint64_t height, uint64_t depth = 1);
            TextureBuilder& format(ECGPUFormat format);
            TextureBuilder& array(uint32_t size);
            TextureBuilder& sample_count(ECGPUSampleCount count);
            TextureBuilder& allow_render_target();
            TextureBuilder& allow_depth_stencil();
            TextureBuilder& allow_readwrite();
            TextureBuilder& allocate_dedicated();
            TextureBuilder& allow_lone();

        protected:
            TextureBuilder(RenderGraph& graph, TextureNode& node);
            RenderGraph&  graph;
            TextureNode&  node;
            CGPUTextureId imported = nullptr;
        };


    private:
        std::vector<PassNode*>     m_passes;
        std::vector<ResourceNode*> m_resources;

        Blackboard*      blackboard = nullptr;
        DependencyGraph* graph      = nullptr;

    };
} // namespace Chandelier
