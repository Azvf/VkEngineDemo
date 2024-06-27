#pragma once

#include "VkCommon.h"

#include "CommandBuffers.h"

namespace Chandelier
{
    class DescriptorPools;
    class Texture;
    class Sampler;

    struct Location
    {
        explicit Location(uint32_t binding, uint32_t set = 0) : binding(binding), set(set) {}

        bool operator==(const Location& other) const { return binding == other.binding && set == other.set; }

        operator uint32_t() const { return binding; }

        /**
         * References to a binding in the descriptor set.
         */
        uint32_t set;    
        uint32_t binding;
    };

    struct Binding
    {
        Location         location;
        VkDescriptorType type;

        VkBuffer     vk_buffer   = VK_NULL_HANDLE;
        VkDeviceSize buffer_size = 0;

        VkBufferView vk_buffer_view = VK_NULL_HANDLE;

        Texture*  texture    = nullptr;
        VkSampler vk_sampler = VK_NULL_HANDLE;

        VkShaderStageFlags shader_stages;

        bool binded = false;

        Binding() : location(0, 0) { }

        bool is_buffer() const
        {
            return (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) || (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }

        bool is_texel_buffer() const { return type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; }

        bool is_image() const
        {
            return (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
                   (texture && type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
                   (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        }
    };

    /**
     * Bindtable is responsible for the resource management of Descriptor.
     * DO NOT release resource when Descriptor is destroyed, it's just a holder
     */
    struct Descriptor
    {
        VKContext* context = nullptr;

        VkDescriptorPool desc_pool = VK_NULL_HANDLE;
        VkDescriptorSet  desc_set  = VK_NULL_HANDLE;
        VkDescriptorSetLayout set_layout  = VK_NULL_HANDLE;

        std::vector<Binding> bindings;

        Descriptor() = default;
        ~Descriptor() = default;

        Descriptor& operator=(const Descriptor& other);
        Descriptor& operator=(Descriptor&& other);

        VkDescriptorSet Handle() const { return desc_set; }

        VkDescriptorPool PoolHandle() const { return desc_pool; }

        bool Valid() { return set_layout && desc_set; }
    };

    class DescriptorTracker : ResourceTracker<Descriptor>
    {
    public:
        explicit DescriptorTracker(std::shared_ptr<VKContext> context) : m_context(context) {}

        virtual ~DescriptorTracker();

        Binding& GetBinding(Location loc);
        
        void     Bind(Buffer* buffer, Location loc, VkShaderStageFlags stages);
        void     Bind(Texture* texture, Location loc, VkShaderStageFlags stages);
        void     Bind(Texture* texture, Sampler* sampler, Location loc, VkShaderStageFlags stages);
        void     BindDescriptorSet(const VkPipelineLayout pipeline_layout, VkPipelineBindPoint pipeline_bind_point);

        Binding& EnsureLocation(Location loc);

        static VkDescriptorSetLayoutBinding CreateLayoutBinding(const Binding& binding);

        VkDescriptorSetLayout GetSetLayout() const { return m_active_desc_layout; }

        void Sync(/*VkDescriptorSetLayout new_layout*/);

        operator VkDescriptorSetLayout() const { return m_active_desc_layout; }

    protected:
        virtual std::shared_ptr<Descriptor> CreateResource() override;

    private:
        VkDescriptorSetLayout CreateLayout();

    private:
        std::shared_ptr<VKContext> m_context;

        std::vector<Binding>  m_bindings;
        VkDescriptorSetLayout m_active_desc_layout = VK_NULL_HANDLE;
    };

    class BindTable
    {
    public:
        explicit BindTable(std::shared_ptr<VKContext> context) : m_context(context) {}
        virtual ~BindTable();

        void Initialize(uint32_t descriptor_size);
        void UnInit();

        static VkDescriptorSetLayoutBinding CreateLayoutBinding(const Binding& binding);
        static VkDescriptorSetLayout        CreateLayout(VkDevice device, const std::vector<Binding>& bindings);

        void Sync();

        void Bind(Buffer* buffer, Location loc, VkShaderStageFlags stages);
        void Bind(Texture* texture, Location loc, VkShaderStageFlags stages);
        void Bind(Texture* texture, Sampler* sampler, Location loc, VkShaderStageFlags stages);
        
        void BindSet(const VkPipelineLayout pipeline_layout, VkPipelineBindPoint pipeline_bind_point);
        
        uint32_t Size() { return m_descriptors.size(); }

        Descriptor& GetDescriptor(uint32_t index) { return m_descriptors[index]; }
        
        const Descriptor& GetDescriptor(uint32_t index) const
        {
            return const_cast<BindTable*>(this)->m_descriptors[index];
        }
        
    private:
        Binding& GetBinding(Location loc);

    private:
        std::shared_ptr<VKContext> m_context;

        std::vector<Descriptor> m_descriptors;
    };

    class MergedBindTable
    {
    public:
        explicit MergedBindTable(std::shared_ptr<VKContext> context, const BindTable& bind_table_ref) 
            : m_context(context), m_bind_table_ref(bind_table_ref)
        {}
        virtual ~MergedBindTable();

        void Initialize();
        void UnInit();

        void Merge(std::vector<BindTable> bind_tables);

        void BindSet(const VkPipelineLayout pipeline_layout, VkPipelineBindPoint pipeline_bind_point);

    private:
        std::shared_ptr<VKContext> m_context;
        
        uint32_t m_set_count = 0;
        
        const BindTable& m_bind_table_ref;

        std::vector<Descriptor> m_copied;
        std::vector<Descriptor> m_merged;
        std::vector<Descriptor> m_results;
    };

} // namespace Chandelier