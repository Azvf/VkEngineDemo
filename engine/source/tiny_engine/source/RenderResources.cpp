#include "RenderResources.h"

#include "Image.h"
#include "Buffer.h"
#include "SwapChain.h"

namespace vulkan {
	RenderResources::RenderResources(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool cmdPool, uint32_t width, uint32_t height, SwapChain* swapChain)
		: m_physicalDevice(physicalDevice), m_device(device), m_commandPool(cmdPool), m_swapChain(swapChain)
	{
		


	}
}