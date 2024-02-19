#pragma once

#include "VkCommon.h"

namespace Chandelier
{
    class Shader
    {
    public:
        Shader() = default;
        ~Shader();

        void Initialize(std::shared_ptr<VKContext> context, const uint8_t* code, uint64_t size);
        void UnInit();

        bool IsInitialized() const { return m_shader_module != VK_NULL_HANDLE; }

        const VkShaderModule& GetModule() const { return m_shader_module; }

    private:
        VkShaderModule m_shader_module = VK_NULL_HANDLE;

        std::shared_ptr<VKContext> m_context;
    };

} // namespace Chandelier