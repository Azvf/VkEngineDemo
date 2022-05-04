#include "Texture.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "VkUtil.h"

namespace vulkan {

	std::shared_ptr<Texture> Texture::load(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool cmdPool, const char* path, bool cube)
	{
		std::shared_ptr<Texture> texture = std::make_shared<Texture>();

		// read image file
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		// fill out info values
		texture->m_device = device;
		texture->m_imageType = VK_IMAGE_TYPE_2D;
		texture->m_format = VK_FORMAT_R8G8B8A8_SRGB;
		texture->m_width = texWidth;
		texture->m_height = texHeight;
		texture->m_depth = 1;
		texture->m_mipLevels = 1;
		texture->m_arrayLayers = 1;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = imageSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		createBuffer(physicalDevice, device, bufferInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		stbi_image_free(pixels);

		VkImageCreateInfo imageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageCreateInfo.flags = cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
		imageCreateInfo.imageType = texture->m_imageType;
		imageCreateInfo.format = texture->m_format;
		imageCreateInfo.extent.width = texture->m_width;
		imageCreateInfo.extent.height = texture->m_height;
		imageCreateInfo.extent.depth = texture->m_depth;
		imageCreateInfo.mipLevels = texture->m_mipLevels;
		imageCreateInfo.arrayLayers = texture->m_arrayLayers;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		createImage(
			physicalDevice, device,
			imageCreateInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			texture->m_image, texture->m_deviceMemory
		);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = texture->m_image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = texture->m_format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &viewInfo, nullptr, &texture->m_view) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		transitionImageLayout(device, queue, cmdPool, texture->m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(device, queue, cmdPool, stagingBuffer, texture->m_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		transitionImageLayout(device, queue, cmdPool, texture->m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		return texture;
	}

	Texture::~Texture()
	{
		vkDestroyImageView(m_device, m_view, nullptr);
		vkDestroyImage(m_device, m_image, nullptr);
		vkFreeMemory(m_device, m_deviceMemory, nullptr);
	}

	VkImageView Texture::getView() const
	{
		return m_view;
	}

	VkImage Texture::getImage() const
	{
		return m_image;
	}

	VkImageType Texture::getType() const
	{
		return m_imageType;
	}

	VkFormat Texture::getFormat() const
	{
		return m_format;
	}

	uint32_t Texture::getWidth() const
	{
		return m_width;
	}

	uint32_t Texture::getHeight() const
	{
		return m_height;
	}

	uint32_t Texture::getDepth() const
	{
		return m_depth;
	}

	uint32_t Texture::getLevels() const
	{
		return m_mipLevels;
	}

	uint32_t Texture::getLayers() const
	{
		return m_arrayLayers;
	}

}