#include "global_context.h"

#include "UI/window_system.h"
#include "render/base/render_system.h"
#include "resource/asset_manager/asset_manager.h"
#include "runtime/framework/config_manager/config_manager.h"

namespace Chandelier
{
    RuntimeGlobalContext g_context;

    void RuntimeGlobalContext::StartSystems(std::string_view config_path)
    {
        m_config_manager = std::make_shared<ConfigManager>();
        m_config_manager->Initialize("config file path"); // todo         
        m_asset_manager  = std::make_shared<AssetManager>();
    }

    void RuntimeGlobalContext::ShutdownSystems() {}

} // namespace Chandelier