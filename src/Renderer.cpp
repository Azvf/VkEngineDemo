#include "Renderer.h"

#include <chrono>
#include <iostream>
#include <unordered_map>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <tinyobjloader/tiny_obj_loader.h>

#include "VkUtil.h"
#include "Descriptor.h"
#include "Mesh.h"
#include "Texture.h"
#include "Pipeline.h"

namespace std {
    template<> struct hash<vulkan::Vertex> {
        size_t operator()(vulkan::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

namespace vulkan {
    Renderer::Renderer(void* windowHandle, uint32_t width, uint32_t height) :
        m_width(width), m_height(height),
        m_context((GLFWwindow*)windowHandle),
        m_sampler(m_context.getPhysicalDevice(), m_context.getDevice()),
        m_uniform(m_context.getPhysicalDevice(), m_context.getDevice()),
        m_swapChain(m_context.getPhysicalDevice(), m_context.getDevice(), m_context.getSurface(), m_width, m_height)
    {
        createAlbedoTexture("../../assets/viking_room/viking_room.png");
        loadObjModel("../../assets/viking_room/viking_room.obj");

        createDescriptor();
        createPipleline();

        createCommandBuffers();
        createSyncObjects();
    }

    void Renderer::createAlbedoTexture(const char* path) {
        m_texture = Texture::load(m_context.getPhysicalDevice(), m_context.getDevice(), m_context.getGraphicsQueue(), m_context.getGraphicsCommandPool(), path);
    }

    void Renderer::createSyncObjects() {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(m_context.getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_context.getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_context.getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create semaphores!");
            }
        }
    }

    Renderer::~Renderer()
    {

    }

    void Renderer::createCommandBuffers() {
        m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_context.getGraphicsCommandPool();
        /**
        * @level:
        *   VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
        *   VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
        */
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

        if (vkAllocateCommandBuffers(m_context.getDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

    }

    void Renderer::loadObjModel(const char* path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        
        vertices.clear();
        indices.clear();

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
        
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
        
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
        
                vertex.color = { 1.0f, 1.0f, 1.0f };
        
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
        
                indices.push_back(uniqueVertices[vertex]);
            }
        }

        // TODO: why non-member variable will cause a link error
        m_mesh = Mesh::load(m_context.getPhysicalDevice(), m_context.getDevice(), m_context.getGraphicsQueue(), m_context.getGraphicsCommandPool(), vertices, indices);
    }

    void Renderer::Render(uint32_t width, uint32_t height) {
        /**
        * Frame Rendering Steps:
        *   Wait for the previous frame to finish
        *   Acquire an image from the swap chain
        *   Record a command buffer which draws the scene onto that image
        *   Submit the recorded command buffer
        *   Present the swap chain image
        */

        if ((width != m_width) || (height != m_height)) {
            m_width = width;
            m_height = height;
            framebufferResized = true;
        }
        
        VkDevice device = m_context.getDevice();

        vkWaitForFences(device, 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            m_swapChain.recreate(m_width, m_height);
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &m_inFlightFences[currentFrame]);
        updateUniformBuffer(currentFrame);

        vkResetCommandBuffer(m_commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_context.getGraphicsQueue(), 1, &submitInfo, m_inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_swapChain.getSwapchain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        // specify an array of VkResult values to check for every individual swap chain if presentation was successful
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(m_context.getPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            m_swapChain.recreate(m_width, m_height);
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::recordCommandBuffer() {
        VkCommandBuffer commandBuffer = m_commandBuffers[currentFrame];

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        /**
        * @flags:
        *   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
        *   VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
        *   VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
        */
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        /**
        * If the command buffer was already recorded once,
        * then a call to vkBeginCommandBuffer will implicitly reset it.
        * It's not possible to append commands to a buffer at a later time.
        */
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain.getRenderPass();
        renderPassInfo.framebuffer = m_swapChain.getFramebuffer(currentFrame);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChain.getExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        /**
        * VK_SUBPASS_CONTENTS_INLINE:
        *    The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
        * VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS:
        *    The render pass commands will be executed from secondary command buffers.
        */
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipeline());

        VkBuffer vertexBuffers[] = { m_mesh->getVertexBuffer() };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_mesh->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipelineLayout(), 0, 1, &m_descriptor->getDescriptorSet(currentFrame), 0, nullptr);
        /**
        * @param1: vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
        * @param2: instanceCount: Used for instanced rendering, use 1 if you're not doing that.
        * @param3: firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
        * @param4: firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
        */
        // vkCmdDraw(commandBuffers, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_mesh->getIndexCount()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void Renderer::updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), m_width / (float)m_height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        void* data;
        vkMapMemory(m_context.getDevice(), m_uniform.getUniformMemory(currentImage), 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(m_context.getDevice(), m_uniform.getUniformMemory(currentImage));
    }

    void Renderer::createDescriptor() {
        m_descriptor = std::make_shared<Descriptor>(m_context.getDevice(), m_uniform, m_texture->getView(), m_sampler);
    }

    void Renderer::createPipleline()
    {
        m_pipeline = std::make_shared<Pipeline>(m_context.getPhysicalDevice(), m_context.getDevice(), m_width, m_height, m_swapChain.getExtent(), m_descriptor->getDescriptorLayout(), m_swapChain.getRenderPass());
    }

}