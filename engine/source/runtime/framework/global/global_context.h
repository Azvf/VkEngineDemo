#pragma once

#include "VkCommon.h"

namespace Chandelier
{
    class AssetManager;
    class ConfigManager;
    class WindowSystem;
    class RenderSystem;

    class RuntimeGlobalContext
    {
    public:
        void StartSystems(std::string_view config_path);
        void ShutdownSystems();

    public:
        std::shared_ptr<AssetManager>  m_asset_manager;
        std::shared_ptr<ConfigManager> m_config_manager;
    };

    extern RuntimeGlobalContext g_context;

} // namespace Chandelier