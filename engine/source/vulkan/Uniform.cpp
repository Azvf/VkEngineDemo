#include "Uniform.h"

#include "RenderCfg.h"
#include "VkUtil.h"
#include "VkContext.h"

namespace Chandelier {
    Uniform::~Uniform() {
        if (m_info.context) {
            const auto& device = m_info.context->getDevice();
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroyBuffer(device, m_uniformBuffers[i], nullptr);
                vkFreeMemory(device, m_uniformBuffersMemory[i], nullptr);
            }
        }
    }

    void Uniform::Initialize(const UniformCreateInfo& info) {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

        VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        createInfo.size = bufferSize;
        createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(info.context->getPhysicalDevice(), info.context->getDevice(), createInfo,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
        }

        this->m_info = info;
    }

    std::shared_ptr<Uniform> Uniform::Create(const UniformCreateInfo& info) {
        auto uniform = std::make_shared<Uniform>();
        uniform->Initialize(info);
        return uniform;
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

    VkBuffer Uniform::getUniformBuffer(uint32_t index) const
    {
        return m_uniformBuffers[index];
    }

    VkDeviceMemory Uniform::getUniformMemory(uint32_t index) const
    {
        return m_uniformBuffersMemory[index];
    }

}