#pragma once

#include <map>
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
#include "Texture.h"

namespace sss {
	class ArcBallCamera;
	class UserInput;
}

namespace Chandelier {
	class Texture;
	class Descriptor;
	class Mesh;
	class Pipeline;
	class Vertex;

	enum TextureType {
		ALBEDO = 0,
		HEIGHT_MAP,
		METALLIC_MAP,
		NORMAL_MAP,
		ROUGHNESS_MAP
	};

	class Renderer {
	public:
		explicit Renderer(void* windowHandle, uint32_t width, uint32_t height);
		~Renderer();

	public:
		void Render(uint32_t width, uint32_t height, sss::UserInput& userInput);
		void Renderer::recordCommandBuffer();

	public:
		void createTexture(const char* path, TextureType texType);
		void loadObjModel(const char* path);
		void updateUniformBuffer(uint32_t currentImage);
	
	private:
		void createDescriptor(TextureType texType);
		void createPipleline();
		void createCommandBuffers();
		
	private:
		VKContext m_context;
		Uniform m_uniform;
		Sampler m_sampler;
		SwapChain m_swapChain;
		SyncResources m_syncResrc;

		std::vector<Vertex>		vertices;
		std::vector<uint32_t>	indices;

		std::map<TextureType, std::shared_ptr<Texture>> m_textures;

		std::shared_ptr<Mesh> m_mesh;
		std::shared_ptr<Descriptor> m_descriptor;
		std::shared_ptr<Pipeline> m_pipeline;

		std::vector<VkCommandBuffer> m_commandBuffers;

		Camera m_camera;
		uint32_t m_width, m_height;
		uint32_t currentFrame = 0;

		bool framebufferResized = false;
		
		float timer = 0.0f;
		float timerSpeed = 0.25f;
	};


}