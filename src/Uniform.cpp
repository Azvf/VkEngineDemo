#include "Uniform.h"

#include "RenderCfg.h"
#include "VkUtil.h"

namespace vulkan {
	Uniform::Uniform(VkPhysicalDevice physicalDevice, VkDevice device)
        : m_physicalDevice(physicalDevice), m_device(device)
    {
        createUniformBuffers();
	}

	void Uniform::createUniformBuffers()
	{
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

        VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        createInfo.size = bufferSize;
        createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(m_physicalDevice, m_device, createInfo,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
        }
	}

    VkBuffer Uniform::getUniformBuffer(uint32_t index)
    {
        return m_uniformBuffers[index];
    }

    VkDeviceMemory Uniform::getUniformMemory(uint32_t index)
    {
        return m_uniformBuffersMemory[index];
    }

}