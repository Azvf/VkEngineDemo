#pragma once

#include "VkCreateInfo.h"

namespace Chandelier
{
    class Image
    {
    public:
        Image() = default;
        ~Image();

        const VkImage&        getImage() const;
        const VkImageView&    getView() const;
        const VkDeviceMemory& getMemory() const;

        void                          Initialize(const ImageCreateInfo& info);
        static std::shared_ptr<Image> Create(const ImageCreateInfo& info);
        static std::shared_ptr<Image> Create(const VkImageCreateInfo& info);
        // friend static std::shared_ptr<Image> Create(VkImage vk_image, VkImageViewCreateInfo view_info);

    private:
        VkImageCreateInfo ToVkImageCreateInfo(const ImageCreateInfo& info);

    private:
        VkImage        m_image;
        VkImageView    m_view;
        VkDeviceMemory m_memory;

        ImageCreateInfo m_info;
    };
} // namespace Chandelier