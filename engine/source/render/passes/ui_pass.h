#pragma once

#include "base/render_pass.h"

namespace Chandelier {

	struct UIPassInitInfo : public BaseRenderPassInitInfo {
		virtual ~UIPassInitInfo() = default;
	};

	class UIPass : public RenderPass {
	public:
		UIPass() = default;
		~UIPass();

	public:
		virtual void Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) override;

	private:
		std::shared_ptr<UIPassInitInfo> m_pass_info;
	};

}