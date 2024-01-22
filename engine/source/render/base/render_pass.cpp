#include "render_pass.h"

#include "render/base/render_resources.h"
#include "Descriptor.h"
#include "Image.h"
#include "Texture.h"

namespace Chandelier {
	VkRenderPass RenderPass::getRenderPass() const {
		return m_framebuffer.render_pass;
	}

	std::vector<VkImageView> RenderPass::getFramebufferImageViews() const {
		std::vector<VkImageView> image_views;
		for (auto& attach : m_framebuffer.attachments) {
			image_views.push_back(attach->getView());
		}
		return image_views;
	}
}

