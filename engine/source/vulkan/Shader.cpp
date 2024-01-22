#include "Shader.h"

#include "runtime/core/base/exception.h"

#include "VkContext.h"

namespace Chandelier {
	Shader::~Shader() {
		if (info.context) {
			vkDestroyShaderModule(info.context->getDevice(), shader_module, nullptr);
		}
	}

	std::shared_ptr<Shader> Shader::Create(const ShaderCreateInfo& info, std::vector<uint8_t>& code) {
		auto shader = std::make_shared<Shader>();
		shader->Initialize(info, code);
		return shader;
	}

	void Shader::Initialize(const ShaderCreateInfo& info, std::vector<uint8_t>& code) {
		const auto& device = info.context->getDevice();

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VULKAN_API_CALL(vkCreateShaderModule(device, &createInfo, nullptr, &shader_module));
		this->info = info;
	}


}