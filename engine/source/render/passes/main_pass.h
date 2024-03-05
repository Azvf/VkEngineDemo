#pragma once

#include <glm/mat4x4.hpp>
#include "base/render_pass.h"
#include "light/light.h"

namespace Chandelier
{
    class Buffer;

    /**
     * @todo: refactor camera class and move it there
     */
    struct CameraUniformBuffer
    {
        glm::mat4 view;
        glm::mat4 projection;

        glm::vec4 position;
        glm::vec4 position_padding_0;
        glm::vec4 position_padding_1;
        glm::vec4 position_padding_2;
    };

    struct ConfigUniformBuffer
    {
        int32_t anti_aliasing;
        int32_t padding;
        int32_t rotating;
        int32_t use_gamma_correction;
        int32_t tone_mapping;
        int32_t display_texture;

        glm::vec2 padding_0;

        int32_t placehodler_setting_0;
        int32_t placehodler_setting_1;
        int32_t placehodler_setting_2;
        int32_t placehodler_setting_3;

        glm::vec4 padding_1;
    };

    /**
     * @info: all structs are 64 bytes aligned
     */
    struct MainPassUniformBuffer
    {
        ConfigUniformBuffer config;
        CameraUniformBuffer camera;
        Lights              lights;
    };

    struct MainRenderPassInitInfo : public BaseRenderPassInitInfo
    {
        virtual ~MainRenderPassInitInfo() = default;

        std::shared_ptr<MainPassUniformBuffer> memory_uniform_buffer;
    };

    class MainRenderPass : public RenderPass
    {
    public:
        MainRenderPass() = default;
        ~MainRenderPass();

    public:
        virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) override;
        virtual void UnInit() override;

        virtual void Recreate() override;

        virtual const VkRenderPass* GetRenderPass() override;

        virtual void PreDrawSetup() override;
        virtual void Draw() override;
        virtual void PostDrawCallback() override;

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
        
        /**
         * @todo: optimize use push constants
         */
        std::shared_ptr<Buffer> m_ubo;
        std::shared_ptr<DescriptorTracker> m_desc_tracker;

        std::vector<VkFramebuffer> m_swapchain_framebuffers;
    };

} // namespace Chandelier