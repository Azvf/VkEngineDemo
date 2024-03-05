#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "base/render_pass.h"

namespace Chandelier
{
    enum SkyboxDisplayIndex : int32_t
    {
        Display_None              = 0,
        Display_Skybox            = 1,
        Display_Skybox_Irradiance = 2,
        Display_Skybox_Prefilter  = 3,
    };

    struct SkyboxPassUniformBuffer
    {
        // glm::vec3 camera_position;
        // glm::mat4 view_projection_mat;
        glm::mat4 inv_model_view_projection;
        int32_t   show_skybox_index;
        float     skybox_prefilter_mip_level;
    };

    struct SkyboxPassInitInfo : public BaseRenderPassInitInfo
    {
        virtual ~SkyboxPassInitInfo() = default;

        const VkRenderPass* render_pass = nullptr;
        std::shared_ptr<MainPassUniformBuffer> main_pass_uniform_buffer;
    };

    class SkyboxPass : public RenderPass
    {
    public:
        SkyboxPass() = default;
        virtual ~SkyboxPass();

    public:
        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) override;
        virtual void UnInit() override;
        
        virtual void Recreate() override;

        virtual const VkRenderPass* GetRenderPass() override;

        virtual void PreDrawSetup() override;
        virtual void Draw() override;
        virtual void PostDrawCallback() override;

        void UpdateUniformBuffer(const SkyboxPassUniformBuffer& uniform_buffer);

    private:
        void SetupShaderResources();
        void ResetShaderResources();

        void SetupDescriptorSets();
        void SyncDescriptorSets();
        void ResetDescriptorSets();

        void SetupPipeline();
        void ResetPipeline();

    private:
        std::shared_ptr<SkyboxPassInitInfo> m_pass_info;
        std::shared_ptr<Buffer>             m_ubo;
        std::shared_ptr<DescriptorTracker>  m_desc_tracker;
    };

} // namespace Chandelier