#pragma once

#include <vector>
#include <memory>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "RenderCfg.h"
#include "VkContext.h"
#include "SwapChain.h"
#include "Uniform.h"
#include "Sampler.h"
#include "SyncResources.h"
#include "Camera.h"

namespace sss {
	class ArcBallCamera;
	class UserInput;
}

namespace vulkan {
	class Texture;
	class Descriptor;
	class Mesh;
	class Pipeline;
	class Vertex;

	class Renderer {
	public:
		explicit Renderer(void* windowHandle, uint32_t width, uint32_t height);
		~Renderer();

	public:
		void Render(uint32_t width, uint32_t height, sss::UserInput& userInput);
		void Renderer::recordCommandBuffer();

	public:
		void createAlbedoTexture(const char* path);
		void loadObjModel(const char* path);
		void updateUniformBuffer(uint32_t currentImage);
	
	private:
		void createDescriptor();
		void createPipleline();
		void createCommandBuffers();
		
	private:
		VKContext m_context;
		Uniform m_uniform;
		Sampler m_sampler;
		SwapChain m_swapChain;
		SyncResources m_syncResrc;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		std::shared_ptr<Texture> m_texture;
		std::shared_ptr<Mesh> m_mesh;
		std::shared_ptr<Descriptor> m_descriptor;
		std::shared_ptr<Pipeline> m_pipeline;

		std::vector<VkCommandBuffer> m_commandBuffers;

		Camera m_camera;
		uint32_t m_width, m_height;
		uint32_t currentFrame = 0;

		bool framebufferResized = false;
	};


}