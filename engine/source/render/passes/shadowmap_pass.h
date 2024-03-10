#pragma once

#include "base/render_pass.h"
#include <glm/mat4x4.hpp>

namespace Chandelier
{
#define SHADOWMAP_DEPTH_FORMAT VK_FORMAT_D32_SFLOAT

    struct ShadowmapPassUniformBuffer
    {
        glm::mat4 view;
        glm::mat4 projection;
    };

    struct ShadowmapPassInitInfo : public BaseRenderPassInitInfo
    {
        virtual ~ShadowmapPassInitInfo() = default;
        const VkRenderPass* render_pass  = nullptr;
        std::shared_ptr<MainPassUniformBuffer> main_pass_uniform_buffer;
    };

    class ShadowmapPass : public RenderPass
    {
    public:
        ShadowmapPass() = default;
        virtual ~ShadowmapPass();

    public:
        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) override;
        virtual void UnInit() override;

        virtual void Recreate() override;

        virtual const VkRenderPass* GetRenderPass() override;

        virtual void PreDrawSetup() override;
        virtual void Draw() override;
        virtual void PostDrawCallback() override;

    public:
        void UpdateUniformBuffer(const ShadowmapPassUniformBuffer& uniform_buffer);

    private:
        void SetupShaderResources();
        void ResetShaderResources();

        void SetupDescriptorSets();
        void SyncDescriptorSets();
        void ResetDescriptorSets();

        void SetupPipeline();
        void ResetPipeline();

    private:
        std::shared_ptr<ShadowmapPassInitInfo>    m_pass_info;
        std::shared_ptr<Buffer>                   m_ubo;
        std::shared_ptr<DescriptorTracker>        m_desc_tracker;
    };

} // namespace Chandelier