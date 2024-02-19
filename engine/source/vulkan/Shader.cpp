#include "Shader.h"

#include "runtime/core/base/exception.h"

#include "VkContext.h"

namespace Chandelier
{
    Shader::~Shader() { UnInit(); }

    void Shader::Initialize(std::shared_ptr<VKContext> context, const uint8_t* code, uint64_t size)
    {
        m_context = context;

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = size;
        createInfo.pCode    = reinterpret_cast<const uint32_t*>(code);
        
        VULKAN_API_CALL(vkCreateShaderModule(context->getDevice(), &createInfo, nullptr, &m_shader_module));
    }

    void Shader::UnInit()
    {
        if (m_shader_module != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(m_context->getDevice(), m_shader_module, nullptr);
        }
        m_shader_module = VK_NULL_HANDLE;
    }

} // namespace Chandelier