#pragma once

#include "VkCommon.h"

namespace Chandelier
{
    class Shader
    {
    public:
        friend class GraphicsPipelineShaders;

        Shader() = default;
        ~Shader();

        void
        Initialize(std::shared_ptr<VKContext> context, VkShaderStageFlagBits stage, const uint8_t* code, uint64_t size);
        void UnInit();

        bool IsInitialized() const { return m_shader_module != VK_NULL_HANDLE; }

        const VkShaderModule& GetModule() const { return m_shader_module; }

        VkShaderStageFlagBits ShaderStage() const { return m_shader_stage; }

    private:
        std::shared_ptr<VKContext> m_context;

        VkShaderModule             m_shader_module = VK_NULL_HANDLE;
        VkShaderStageFlagBits      m_shader_stage  = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    };
    
    class GraphicsPipelineShaders
    {
        struct BindMetaData
        {
            uint32_t           set;
            uint32_t           binding;
            uint32_t           count;
            VkDescriptorType   desc_type;
            VkShaderStageFlags shader_stage_flag;
        };

        struct PushConstMetaData
        {
            uint32_t           size;
            uint32_t           offset;
            VkShaderStageFlags shader_stage_flag;
        };

    public:
        enum ShaderStage : uint8_t
        {
            Vertex_Shader = 0,
            Tessellation_Control_Shader,
            Tessellation_Evaluation_Shader,
            Geometry_Shader,
            Fragment_Shader,
            Shader_Stage_Count
        };

        GraphicsPipelineShaders() = default;
        ~GraphicsPipelineShaders();

        void Initialize(std::shared_ptr<VKContext> context);
        void UnInit();

        void InitShader(std::string_view shader_path, ShaderStage stage);

        std::shared_ptr<Shader> GetShader(ShaderStage shader);

    private:
        VkShaderStageFlagBits ShaderStageToVkStage(ShaderStage stage);

    private:
        std::shared_ptr<VKContext> m_context;

        std::array<std::shared_ptr<Shader>, Shader_Stage_Count> m_shaders;

        std::unordered_map<std::string, BindMetaData>           m_bind_meta_map;
        std::optional<PushConstMetaData>                        m_push_const_data;
    };

} // namespace Chandelier