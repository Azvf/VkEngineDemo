#pragma once

#include <filesystem>

namespace Chandelier
{
    class ConfigManager
    {
    public:
        void Initialize(const std::filesystem::path& config_file_path);

        const std::filesystem::path& GetRootFolder() const { return m_root_folder; }
        const std::filesystem::path& GetAssetFolder() const { return m_asset_folder; }

    private:
        std::filesystem::path m_root_folder;
        std::filesystem::path m_asset_folder;
    };
} // namespace Chandelier
