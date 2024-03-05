#pragma once

#include "base/render_pass.h"
#include <glm/mat4x4.hpp>

namespace Chandelier
{
    struct IrradianceConvolutionPassUniformBuffer
    {
        glm::mat4 view;
        glm::mat4 projection;
    };

    struct IrradianceConvolutionPassInitInfo : public BaseRenderPassInitInfo
    {
        virtual ~IrradianceConvolutionPassInitInfo() = default;
    };

    class IrradianceConvolutionPass : public RenderPass
    {
    public:
        IrradianceConvolutionPass() = default;
        virtual ~IrradianceConvolutionPass();

    public:
        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) override;
        virtual void UnInit() override;

        virtual void Recreate() override;

        virtual const VkRenderPass* GetRenderPass() override;

        virtual void PreDrawSetup() override;
        virtual void Draw() override;
        virtual void PostDrawCallback() override;

        void UpdateUniformBuffer(const IrradianceConvolutionPassUniformBuffer& uniform_buffer);

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
        std::shared_ptr<IrradianceConvolutionPassInitInfo> m_pass_info;
        std::shared_ptr<Buffer>             m_ubo;
        std::shared_ptr<DescriptorTracker>  m_desc_tracker;
    };

} // namespace Chandelier