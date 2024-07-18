#pragma once

#include <memory>
#include <stdint.h>
#include <string>
#include <span>
#include <utility>

#include "dependency_graph.h"

#include "vulkan/vulkan_core.h"

namespace Chandelier
{
    using RHandle = uint64_t;

    class VKContext;
    class Texture;
    class Buffer;
    class RenderGraphBackend;
    class PassNode;
    class RenderGraphFrameExecutor;

    enum ERenderResourceType : uint8_t
    {
        None_Resource = 0,
        Render_Pass_Resource,
        Texture_Resource,
        Buffer_Resource,
        Resource_Count
    };

    enum EEdgeType : uint8_t
    {
        Node_Edge_Type = 0,
        R_Texture,       // SRV
        W_Texture,       // RTV/DSV
        RW_Texture,      // UAV
        Pipeline_Buffer, // VB/IB...
        R_Buffer,        // CBV
        RW_Buffer,       // UAV
        Edge_Type_Count
    };

    enum EPassType : uint8_t
    {
        None_Pass = 0,
        Render_Pass,
        Compute_Pass,
        Copy_Pass,
        Present_Pass,
        Pass_Count
    };

    enum
    {
        Render_Graph_Invalid_Resource_Tag = 1,
        Render_Graph_Default_Resource_Tag = 1 << 1,
        Render_Graph_Dynamic_Resource_Tag = 1 << 2,
    };

    template<ERenderResourceType type>
    struct ResourceHandle {
    
        ResourceHandle(RHandle h) : m_handle(h) {}
        RHandle Handle() { return m_handle; }

    private:
        RHandle m_handle;
    };

    template<>
    struct ResourceHandle<Buffer_Resource>
    {
        ResourceHandle() {}

        RHandle Handle() { return m_handle; }

        struct ShaderReadHandle
        {
            friend struct ResourceHandle<Buffer_Resource>;
            friend class RenderGraph;
            friend class BufferReadEdge;
            const RHandle handle;
            inline operator ResourceHandle<Buffer_Resource>() const { return ResourceHandle<Buffer_Resource>(handle); }

        protected:
            ShaderReadHandle(const RHandle h);
        };

        struct ShaderReadWriteHandle
        {
            friend struct ResourceHandle<Buffer_Resource>;
            friend class RenderGraph;
            friend class BufferReadWriteEdge;
            const RHandle handle;
            inline operator ResourceHandle<Buffer_Resource>() const { return ResourceHandle<Buffer_Resource>(handle); }

        protected:
            ShaderReadWriteHandle(const RHandle h);
        };

        struct RangeHandle
        {
            friend struct ResourceHandle<Buffer_Resource>;
            friend class RenderGraph;
            friend class BufferReadEdge;
            const RHandle  handle;
            const uint64_t from;
            const uint64_t to;

            inline operator ResourceHandle<Buffer_Resource>() const { return ResourceHandle<Buffer_Resource>(handle); }

        protected:
            inline RangeHandle(const RHandle h, uint64_t from, uint64_t to) : handle(h), from(from), to(to) {}
        };

        struct PipelineReferenceHandle
        {
            friend struct ResourceHandle<Buffer_Resource>;
            friend class RenderGraph;
            friend class PipelineBufferEdge;
            const RHandle handle;
            inline operator ResourceHandle<Buffer_Resource>() const { return ResourceHandle<Buffer_Resource>(handle); }

        protected:
            PipelineReferenceHandle(const RHandle h);
        };

        // read
        inline operator ShaderReadHandle() const { return ShaderReadHandle(m_handle); }
        // readwrite
        inline operator ShaderReadWriteHandle() const { return ShaderReadWriteHandle(m_handle); }
        // pipeline
        inline operator PipelineReferenceHandle() const { return PipelineReferenceHandle(m_handle); }
        // range
        inline RangeHandle range(uint64_t from, uint64_t to) const { return RangeHandle(m_handle, from, to); }

    private:
        ResourceHandle(RHandle h) : m_handle(h) {}

        RHandle m_handle = UINT64_MAX;
    };

    using BufferHandle = ResourceHandle<Buffer_Resource>;
    using BufferCBVHandle = BufferHandle::ShaderReadHandle;
    using BufferUAVHandle = BufferHandle::ShaderReadWriteHandle;
    using BufferRangeHandle = BufferHandle::RangeHandle;
    using PipelineBufferHandle = BufferHandle::PipelineReferenceHandle;

    
    template<>
    struct ResourceHandle<Texture_Resource>
    {
        struct SubresourceHandle
        {
            friend struct ResourceHandle<Texture_Resource>;
            friend class RenderGraph;
            friend class RenderGraphBackend;
            friend class TextureCopyEdge;
            inline operator ResourceHandle<Texture_Resource>() const
            {
                return ResourceHandle<Texture_Resource>(handle);
            }

            SubresourceHandle(const RHandle h);

        protected:
            RHandle               handle;
            uint32_t              mip_level   = 0;
            uint32_t              array_base  = 0;
            uint32_t              array_count = 1;
            VkImageAspectFlagBits aspects     = VK_IMAGE_ASPECT_COLOR_BIT;
        };

        struct ShaderReadHandle
        {
            friend struct ResourceHandle<Texture_Resource>;
            friend class RenderGraph;
            friend class TextureReadEdge;
            ShaderReadHandle read_mip(uint32_t base, uint32_t count) const;
            ShaderReadHandle read_array(uint32_t base, uint32_t count) const;
            ShaderReadHandle dimension(VkImageViewType dim) const;
            inline           operator ResourceHandle<Texture_Resource>() const
            {
                return ResourceHandle<Texture_Resource>(handle);
            }

            ShaderReadHandle(const RHandle  h,
                             const uint32_t mip_base    = 0,
                             const uint32_t mip_count   = 1,
                             const uint32_t array_base  = 0,
                             const uint32_t array_count = 1);

        protected:
            RHandle         handle;
            uint32_t        mip_base    = 0;
            uint32_t        mip_count   = 1;
            uint32_t        array_base  = 0;
            uint32_t        array_count = 1;
            VkImageViewType dim         = VK_IMAGE_VIEW_TYPE_2D;
        };

        struct ShaderWriteHandle
        {
            friend struct ResourceHandle<Texture_Resource>;
            friend class RenderGraph;
            friend class TextureRenderEdge;
            ShaderWriteHandle write_mip(uint32_t mip_level) const;
            ShaderWriteHandle write_array(uint32_t base, uint32_t count) const;
            inline            operator ResourceHandle<Texture_Resource>() const
            {
                return ResourceHandle<Texture_Resource>(handle);
            }

            ShaderWriteHandle(const RHandle _this);

        protected:
            RHandle  handle;
            uint32_t mip_level   = 0;
            uint32_t array_base  = 0;
            uint32_t array_count = 1;
        };

        struct DepthStencilHandle : public ShaderWriteHandle
        {
            friend struct ResourceHandle<Texture_Resource>;
            friend class RenderGraph;
            friend class TextureRenderEdge;

            DepthStencilHandle clear_depth(float depth) const;

        protected:
            inline DepthStencilHandle(const RHandle h) : ShaderWriteHandle(h) {}

            float cleardepth = 0.f;
        };

        struct ShaderReadWriteHandle
        {
            friend struct ResourceHandle<Texture_Resource>;
            friend class RenderGraph;
            friend class TextureReadWriteEdge;
            inline operator ResourceHandle<Texture_Resource>() const
            {
                return ResourceHandle<Texture_Resource>(handle);
            }

            ShaderReadWriteHandle(const RHandle h);

        protected:
            RHandle handle;
        };

        inline operator RHandle() const { return m_handle; }
        // read
        inline           operator ShaderReadHandle() const { return ShaderReadHandle(m_handle); }
        ShaderReadHandle read_mip(uint32_t base, uint32_t count) const;
        ShaderReadHandle read_array(uint32_t base, uint32_t count) const;
        // write
        inline            operator ShaderWriteHandle() const { return ShaderWriteHandle(m_handle); }
        ShaderWriteHandle write_mip(uint32_t mip_level) const;
        ShaderWriteHandle write_array(uint32_t base, uint32_t count) const;
        // readwrite
        inline operator ShaderReadWriteHandle() const { return ShaderReadWriteHandle(m_handle); }
        // ds
        inline             operator DepthStencilHandle() const { return DepthStencilHandle(m_handle); }
        DepthStencilHandle clear_depth(float depth) const;
        // subresource
        inline operator SubresourceHandle() const { return SubresourceHandle(m_handle); }
        friend class RenderGraph;
        friend class RenderGraphBackend;
        friend class TextureNode;
        friend class TextureReadEdge;
        friend class TextureRenderEdge;
        friend struct ShaderReadHandle;
        friend struct ShaderWriteHandle;
        friend struct ShaderReadWriteHandle;
        friend struct SubresourceHandle;
        ResourceHandle() {};

    private:
        ResourceHandle(RHandle h) : m_handle(h) {}

        RHandle m_handle = UINT64_MAX;
    };

    using PassHandle = ResourceHandle<Render_Pass_Resource>;
    using TextureHandle = ResourceHandle<Texture_Resource>;
    using TextureSRVHandle = TextureHandle::ShaderReadHandle;
    using TextureRTVHandle = TextureHandle::ShaderWriteHandle;
    using TextureDSVHandle = TextureHandle::DepthStencilHandle;
    using TextureUAVHandle = TextureHandle::ShaderReadWriteHandle;
    using TextureSubresourceHandle = TextureHandle::SubresourceHandle;


    class RenderGraphNode : public DependencyGraphNode
    {
    public:
        RenderGraphNode(ERenderResourceType type);

        void        SetName(std::string_view name);
        std::string GetName() const;

    protected:
        ERenderResourceType m_rtype;
        std::string         m_name;
        uint32_t            m_pooled_size = {};
    };

    
    class RenderGraphEdge : public DependencyGraphEdge
    {
    public:
        RenderGraphEdge(EEdgeType type);

    protected:
        EEdgeType m_etype;
        uint32_t  m_pooled_size = {};
    };


    struct RenderPassContext
    {
        std::shared_ptr<PassNode>                                       pass  = nullptr;
        std::shared_ptr<RenderGraphBackend>                             graph = nullptr;
        std::shared_ptr<VKContext>                                      vkctx = nullptr;
        std::vector<std::pair<BufferHandle, std::shared_ptr<Buffer>>>   resolved_buffers;
        std::vector<std::pair<TextureHandle, std::shared_ptr<Texture>>> resolved_textures;

        std::shared_ptr<Buffer>  resolve(BufferHandle buffer_handle) const;
        std::shared_ptr<Texture> resolve(TextureHandle tex_handle) const;
    };

    struct BindablePassContext : public RenderPassContext
    {
        friend class RenderGraphBackend;

        const struct CGPUXBindTable*       create_and_update_bind_table() ;
        const struct CGPUXMergedBindTable* merge_tables() ;

        const struct CGPUXBindTable* bind_table;

    protected:
        class RenderGraphFrameExecutor* executor;
    };


} // namespace Chandelier