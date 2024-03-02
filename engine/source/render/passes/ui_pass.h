#pragma once

#include "base/render_pass.h"

namespace Chandelier {
    class WindowSystem;
	
	struct UIPassInitInfo : public BaseRenderPassInitInfo {
		virtual ~UIPassInitInfo() = default;

		std::shared_ptr<WindowSystem> window_system;
        std::shared_ptr<MainPassUniformBuffer> main_pass_uniform_buffer;

        const VkRenderPass*           render_pass = nullptr;
	};

	class UIPass : public RenderPass {
	public:
		UIPass() = default;
		virtual ~UIPass();

	public:
		virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) override;
        virtual void UnInit() override;
        
        virtual void Recreate() override;

        virtual const VkRenderPass* GetRenderPass() override;

        virtual void PreDrawSetup() override;
        virtual void Draw() override;
        virtual void PostDrawCallback() override;
	
	private:
        void ImGuiSetCheckBox(const char* label, int32_t* v);
        void ImGuiDraw();

        // void SetupAttachments();
        // void ResetAttachments();

        // void SetupRenderPass();
        // void ResetRenderPass();

        void SetupImGuiPipelineContext();
        void ResetImGuiPipelineContext();

		// void SetupFramebuffers();
        // void ResetFramebuffers();

	private:
		std::shared_ptr<UIPassInitInfo> m_pass_info;
	};

}