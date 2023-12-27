#include "Renderer.h"

#include <chrono>
#include <iostream>
#include <unordered_map>
#include <filesystem>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <tinyobjloader/tiny_obj_loader.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include "VkUtil.h"
#include "Descriptor.h"
#include "Mesh.h"
#include "Texture.h"
#include "Pipeline.h"
#include "ArcBallCamera.h"
#include "UserInput.h"
#include "common_utils.h"

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
        m_swapChain(m_context.getPhysicalDevice(), m_context.getDevice(), m_context.getSurface(), m_width, m_height),
        m_syncResrc(m_context.getDevice())
    {
        // Setup a default look-at camera
        m_camera.type = Camera::CameraType::lookat;
        m_camera.setPosition(glm::vec3(0.0f, -0.25f, -.5f));
        m_camera.setRotation(glm::vec3(0.0f));
        m_camera.setPerspective(40.0f, width / (float)height, 0.01f, 256.0f);

        auto exe_path = Utils::getCurrentProcessDirectory();
        auto source_path = exe_path.parent_path().parent_path();
        auto tex_path = source_path / "engine/assets/Boat/texture/bench 1_Base_color.png";
        auto obj_model_path = source_path / "engine/assets/Boat/Boat.obj";
        createTexture(tex_path.string().data(), ALBEDO);
        loadObjModel(obj_model_path.string().data());

        createDescriptor(ALBEDO);
        createPipleline();

        createCommandBuffers();
    }

    void Renderer::createTexture(const char* path, TextureType texType) {
        m_textures[texType] = Texture::load(m_context.getPhysicalDevice(), m_context.getDevice(), m_context.getGraphicsQueue(), m_context.getGraphicsCommandPool(), path);
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

    void Renderer::Render(uint32_t width, uint32_t height, sss::UserInput& userInput) {
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
        
        userInput.input();

        VkDevice device = m_context.getDevice();

        vkWaitForFences(device, 1, &m_syncResrc.getInFlightFence(currentFrame), VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, m_swapChain.getSwapchain(), UINT64_MAX, m_syncResrc.getImageAvailSemaphore(currentFrame), VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // m_swapChain.recreate(m_width, m_height);
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &m_syncResrc.getInFlightFence(currentFrame));
        
        const glm::vec2 mouseDelta = userInput.getMousePosDelta();
        const float scrollDelta = userInput.getScrollOffset().y;

        if (userInput.isMouseButtonPressed(InputMouse::BUTTON_LEFT)) {
            m_camera.rotate(glm::vec3(mouseDelta.y * m_camera.rotationSpeed, mouseDelta.x * m_camera.rotationSpeed, 0.0f));
        }

        auto freeRoamBaseSpeed = 0.001f;
        if (userInput.isKeyPressed(InputKey::W)) {
            auto forward = glm::normalize(glm::vec3(m_camera.viewPos));
            m_camera.translate(forward * freeRoamBaseSpeed);
        } 
        else if (userInput.isKeyPressed(InputKey::S)) {
            auto backward = -glm::normalize(glm::vec3(m_camera.viewPos));
            m_camera.translate(backward * freeRoamBaseSpeed);
        } 
        else if (userInput.isKeyPressed(InputKey::A)) {
            auto right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), glm::vec3(m_camera.viewPos)));
            m_camera.translate(right * freeRoamBaseSpeed);
        } 
        else if (userInput.isKeyPressed(InputKey::D)) {
            auto left = -glm::normalize(glm::cross(glm::vec3(0, 1, 0), glm::vec3(m_camera.viewPos)));
            m_camera.translate(left * freeRoamBaseSpeed);
        }
        else if (userInput.isKeyPressed(InputKey::Q)) {
            auto upwards = glm::vec3(0, 1, 0);
            m_camera.translate(upwards * freeRoamBaseSpeed);
        }
        else if (userInput.isKeyPressed(InputKey::E)) {
            auto downwards = glm::vec3(0, -1, 0);
            m_camera.translate(downwards * freeRoamBaseSpeed);
        }
        else if (userInput.isKeyPressed(InputKey::R)) {
            m_camera.setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
            m_camera.setRotation(glm::vec3(0.0f));
        }
        
        updateUniformBuffer(currentFrame);

        vkResetCommandBuffer(m_commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_syncResrc.getImageAvailSemaphore(currentFrame) };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { m_syncResrc.getRenderFinishedSemaphore(currentFrame) };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_context.getGraphicsQueue(), 1, &submitInfo, m_syncResrc.getInFlightFence(currentFrame)) != VK_SUCCESS) {
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

        result = vkQueuePresentKHR(m_context.getPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            // m_swapChain.recreate(m_width, m_height);
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

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipeline());

            VkBuffer vertexBuffers[] = { m_mesh->getVertexBuffer() };
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, m_mesh->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipelineLayout(), 0, 1, &m_descriptor->getDescriptorSet(currentFrame), 0, nullptr);

            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
         vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void Renderer::updateUniformBuffer(uint32_t currentImage) {
        UniformBufferObject ubo{};
        ubo.model = glm::mat4(1.0f);
        ubo.view = m_camera.matrices.view;
        ubo.proj = m_camera.matrices.perspective;
        ubo.proj[1][1] *= -1;

        void* data;
        vkMapMemory(m_context.getDevice(), m_uniform.getUniformMemory(currentImage), 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(m_context.getDevice(), m_uniform.getUniformMemory(currentImage));
    }

    void Renderer::createDescriptor(TextureType texType) {
        m_descriptor = std::make_shared<Descriptor>(m_context.getDevice(), m_context.getDescriptorPool(), m_textures[texType]->getView(), m_uniform, m_sampler);
    }

    void Renderer::createPipleline() {
        m_pipeline = std::make_shared<Pipeline>(m_context.getPhysicalDevice(), m_context.getDevice(), m_width, m_height, m_swapChain.getExtent(), m_descriptor->getDescriptorLayout(), m_swapChain.getRenderPass());
    }

}