#pragma once

#include <glm/mat4x4.hpp>
#include "base/render_pass.h"

namespace Chandelier
{
    class Buffer;
    class DescriptorTracker;

    struct MainPassUniformBuffer
    {
        glm::mat4 model_view;
        glm::mat4 projection;
    };

    enum AntiAliasing : uint8_t
    {
        None_AA     = 0,
        Enable_MSAA = 1,
        Enable_FXAA = 2,
    };

    struct MainRenderPassInitInfo : public BaseRenderPassInitInfo
    {
        virtual ~MainRenderPassInitInfo() = default;
        AntiAliasing aa;
    };

    class MainRenderPass : public RenderPass
    {
    public:
        MainRenderPass() = default;
        ~MainRenderPass();

    public:
        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) override;
        virtual void UnInit() override;

        virtual const VkRenderPass* GetRenderPass() override;

        virtual void PreDrawSetup() override;
        virtual void Draw() override;
        virtual void PostDrawCallback() override;

        void Resize();
        
        void UpdateUniformBuffer(const MainPassUniformBuffer& uniform_buffer);

        void ForwardDraw(std::vector<std::shared_ptr<RenderPass>> subpasses);

    private:
        void SetupUniformBuffer();
        void ResetUniformBuffer();

        void SetupAttachments();
        void ResetAttachments();

        void SetupDescriptorSets();
        void SyncDescriptorSets();
        void ResetDescriptorSets();
        
        void SetupPipeline();
        void ResetPipeline();

        void SetupFramebuffers();
        void ResetFramebuffers();

    private:
        std::shared_ptr<MainRenderPassInitInfo> m_pass_info;
        
        enum LayoutType : uint8_t
        {
            Global_Mesh_Layout = 0,
            Mesh_Material_Layout,
            // Per_Mesh_Layout,
            // Skybox_Layout,
            Layout_Type_Count
        };

        std::vector<std::shared_ptr<Texture>> m_textures;
        std::vector<std::shared_ptr<Mesh>>    m_meshes;
        
        std::shared_ptr<Buffer> m_ubo;
        std::shared_ptr<DescriptorTracker> m_desc_tracker;

        std::vector<VkFramebuffer> m_swapchain_framebuffers;
    };

} // namespace Chandelier