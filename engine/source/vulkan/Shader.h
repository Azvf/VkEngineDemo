#pragma once

#include <memory>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "VkCreateInfo.h"

namespace Chandelier {
	class Shader {
	public:
		Shader() = default;
		~Shader();

		void Initialize(const ShaderCreateInfo& info, std::vector<uint8_t>& code);
		static std::shared_ptr<Shader> Create(const ShaderCreateInfo& info, std::vector<uint8_t>& code);

	private:
		VkShaderModule		shader_module;

		ShaderCreateInfo	info;
	};

}