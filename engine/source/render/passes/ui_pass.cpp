#include "ui_pass.h"

namespace Chandelier {
	UIPass::~UIPass() {

	}

	void UIPass::Initialize(std::shared_ptr<BaseRenderPassInitInfo> info) {
		m_pass_info = std::dynamic_pointer_cast<UIPassInitInfo>(info);


	}

}

