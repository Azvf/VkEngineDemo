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
        Location() = default;
        Location(uint32_t binding) : binding(binding) {}

        bool operator==(const Location& other) const { return binding == other.binding; }

        operator uint32_t() const { return binding; }

        /**
         * References to a binding in the descriptor set.
         */
        uint32_t binding;
        
        // friend class VKDescriptorSetTracker;
        // friend class VKShaderInterface;
        // friend class Binding;
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

        Binding() { location.binding = 0; }

        bool is_buffer() const
        {
            return (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) || (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }

        bool is_texel_buffer() const { return type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; }

        bool is_image() const
        {
            return (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
                   (texture && type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        }
    };

    /**
     * In vulkan shader resources (images and buffers) are grouped in descriptor sets.
     *
     * The resources inside a descriptor set can be updated and bound per set.
     *
     * Currently Blender only supports a single descriptor set per shader, but it is planned to be
     * able to use 2 descriptor sets per shader. One for each #blender::gpu::shader::Frequency.
     */
    struct Descriptor
    {
        VKContext* m_context;

        VkDescriptorPool m_desc_pool = VK_NULL_HANDLE;
        VkDescriptorSet  m_desc_set  = VK_NULL_HANDLE;

        Descriptor() = default;
        Descriptor(VKContext* context, VkDescriptorPool pool, VkDescriptorSet set) :
            m_context(context), m_desc_pool(pool), m_desc_set(set)
        {}
        Descriptor(Descriptor&& other);
        ~Descriptor();

        Descriptor& operator=(Descriptor&& other)
        {
            assert(other.m_desc_set != VK_NULL_HANDLE);

            m_context   = other.m_context;
            m_desc_set  = other.m_desc_set;
            m_desc_pool = other.m_desc_pool;

            other.m_context   = nullptr;
            other.m_desc_set  = VK_NULL_HANDLE;
            other.m_desc_pool = VK_NULL_HANDLE;

            return *this;
        }

        VkDescriptorSet Handle() const { return m_desc_set; }

        VkDescriptorPool PoolHandle() const { return m_desc_pool; }
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

} // namespace Chandelier