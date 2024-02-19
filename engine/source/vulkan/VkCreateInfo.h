#pragma once

#include <memory>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_core.h>

namespace Chandelier
{
    class VKContext;
    class Uniform;
    class Texture;
    class Sampler;

    struct BufferCreateInfo
    {
        VkDeviceSize          size;
        VkBufferCreateInfo    vk_buffer_info;
        VkMemoryPropertyFlags mem_flags;

        std::shared_ptr<VKContext> context;
    };

    struct TextureCreateInfo
    {
        uint32_t              width;
        uint32_t              height;
        VkFormat              format;
        VkImageTiling         tiling;
        VkImageUsageFlags     usage_flags;
        VkMemoryPropertyFlags mem_flags;
        VkImageCreateFlags    create_flags;
        uint32_t              array_layers;
        uint32_t              mip_levels;
        VkImageLayout         layout;

        std::shared_ptr<VKContext> context;
    };

    struct DescriptorCreateInfo
    {
        std::shared_ptr<Texture> texture;
        std::shared_ptr<Uniform> uniform;
        std::shared_ptr<Sampler> sampler;

        std::shared_ptr<VKContext> context;
    };

    struct ShaderCreateInfo
    {

        std::shared_ptr<VKContext> context;
    };

    struct UniformCreateInfo
    {

        std::shared_ptr<VKContext> context;
    };

    struct SamplerCreateInfo
    {

        std::shared_ptr<VKContext> context;
    };

    struct ImageViewInfo
    {
        VkFormat           format;
        VkImageAspectFlags aspect_flags;
    };

    struct ImageCreateInfo
    {
        VkImageCreateInfo     image_info;
        ImageViewInfo         view_info;
        VkMemoryPropertyFlags mem_flags;

        std::shared_ptr<VKContext> context;
    };

} // namespace Chandelier
