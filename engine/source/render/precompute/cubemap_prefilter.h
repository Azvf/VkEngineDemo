#pragma once

#include <glm/mat4x4.hpp>
#include "base/render_pass.h"

namespace Chandelier
{
#define CUBEMAP_PREFILTER_BASE_WIDTH    256
#define CUBEMAP_PREFILTER_BASE_HEIGHT   256
#define CUBEMAP_PREFILTER_MIP_LEVEL 5

    class IrradianceConvolutionPass;

    struct CubemapFilterPassUniformBuffer
    {
        glm::mat4 view;
        glm::mat4 projection;
        float roughness;
    };

    struct CubeMapPrefilterInitInfo : public BaseRenderPassInitInfo
    {
        virtual ~CubeMapPrefilterInitInfo() = default;
    };

    class CubeMapPrefilterPass : public RenderPass
    {
    public:
        CubeMapPrefilterPass() = default;
        virtual ~CubeMapPrefilterPass();

    public:
        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) override;
        virtual void UnInit() override;

        virtual void Recreate() override;

        virtual const VkRenderPass* GetRenderPass() override;

        virtual void PreDrawSetup() override;
        virtual void Draw() override;
        virtual void PostDrawCallback() override;

    public:
        void UpdateUniformBuffer(const CubemapFilterPassUniformBuffer& uniform_buffer);

    private:
        void SetupAttachments();
        void ResetAttachments();

        void SetupShaderResources();
        void ResetShaderResources();

        void SetupDescriptorSets();
        void SyncDescriptorSets();
        void ResetDescriptorSets();

        void SetupPipeline();
        void ResetPipeline();

        void SetupFramebuffers();
        void ResetFramebuffers();

    private:
        std::shared_ptr<CubeMapPrefilterInitInfo> m_pass_info;
        std::shared_ptr<Buffer>                   m_ubo;
        std::shared_ptr<DescriptorTracker>        m_desc_tracker;
    };

} // namespace Chandelier