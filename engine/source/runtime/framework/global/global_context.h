#pragma once

#include "VkCommon.h"
#include "common_utils.h"

namespace Chandelier
{
    class AssetManager;
    class ConfigManager;
    class WindowSystem;
    class RenderSystem;

    struct GlobalContextInitInfo
    {
        std::string executable_path;
        std::string config_file_path;
    };

    #define g_context RuntimeGlobalContext::GetInstance()
    SINGLETON(RuntimeGlobalContext)
    {
    public:
        RuntimeGlobalContext() = default;

        void Initialize(const GlobalContextInitInfo& info);
        void UnInit();

        const fs::path& GetRootFolder() const { return m_root_folder; }
        const fs::path& GetAssetFolder() const { return m_asset_folder; }
        const fs::path& GetShaderFolder() const { return m_shader_folder; }

    public:
        std::shared_ptr<AssetManager>  m_asset_manager;
        std::shared_ptr<ConfigManager> m_config_manager;

    private:
        fs::path m_root_folder;
        fs::path m_asset_folder;
        fs::path m_shader_folder;
    };

} // namespace Chandelier