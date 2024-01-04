#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include "RenderCfg.h"

namespace Chandelier {
	class SwapChain;
	class VKContext;
	class Image;
	class Buffer;
	class Descriptor;
	class Sampler;

	class MainRenderPass;
	class UIPass;

	struct RenderResource {
	public:
		std::shared_ptr<VKContext> m_context;

		// images
		std::unique_ptr<Image> m_depthStencilImage[MAX_FRAMES_IN_FLIGHT];
		std::unique_ptr<Image> m_colorImage[MAX_FRAMES_IN_FLIGHT];
		std::unique_ptr<Image> m_diffuseImage[MAX_FRAMES_IN_FLIGHT];
		std::unique_ptr<Image> m_tonemappedImage[MAX_FRAMES_IN_FLIGHT];

		// framebuffers
		VkFramebuffer				m_shadowFramebuffers	[MAX_FRAMES_IN_FLIGHT];
		VkFramebuffer				m_mainFramebuffers		[MAX_FRAMES_IN_FLIGHT];
		std::vector<VkFramebuffer>	m_guiFramebuffers;

		// const buffers
		std::unique_ptr<Buffer>	m_constantBuffer[MAX_FRAMES_IN_FLIGHT];

		// depth image view
		VkImageView	m_depthImageView[MAX_FRAMES_IN_FLIGHT];

		// render passes
		std::unique_ptr<MainRenderPass> m_mainPass;
		std::unique_ptr<UIPass>			m_uiPass;

		// descriptors
		std::unique_ptr<Descriptor> m_texDescriptor;
		std::unique_ptr<Descriptor> m_lightingDescriptor;
		std::unique_ptr<Descriptor> m_postProcDescriptor;

		// samplers
		std::unique_ptr<Sampler> m_shadowSampler;
		std::unique_ptr<Sampler> m_linearSamplerClamp;
		std::unique_ptr<Sampler> m_linearSamplerRepeat;
		std::unique_ptr<Sampler> m_pointSamplerClamp;
		std::unique_ptr<Sampler> m_pointSamplerRepeat;

	public:
		void Resize(int32_t width, int32_t height);

	private:

	};

}
