#pragma once

#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace vulkan {
	class SyncResources {
	public:
		SyncResources(VkDevice device);
		~SyncResources();

	public:
		const VkSemaphore& getImageAvailSemaphore(uint32_t index) const;
		const VkSemaphore& getRenderFinishedSemaphore(uint32_t index) const;
		const VkFence& getInFlightFence(uint32_t index) const;

	private:
		VkDevice m_device;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
	};


}