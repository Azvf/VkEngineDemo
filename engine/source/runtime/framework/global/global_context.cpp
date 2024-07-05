#include "global_context.h"

#include "resource/asset_manager/asset_manager.h"
#include "runtime/framework/config_manager/config_manager.h"

namespace Chandelier
{
    void RuntimeGlobalContext::Initialize(const GlobalContextInitInfo& info) {
        m_config_manager = std::make_shared<ConfigManager>();
        m_config_manager->Initialize(info.config_file_path);
        
        m_asset_manager = std::make_shared<AssetManager>();

        m_root_folder = fs::canonical(fs::path(info.executable_path)).parent_path();
        m_asset_folder = m_root_folder / "assets";
        m_shader_folder = m_root_folder / "shaders";
    }

    void RuntimeGlobalContext::UnInit() {

    }

} // namespace Chandelier