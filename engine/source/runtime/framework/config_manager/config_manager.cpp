#include "config_manager.h"

namespace Chandelier
{
    void ConfigManager::Initialize(const std::filesystem::path& config_file_path)
    {
        m_root_folder  = "";
        m_asset_folder = "";
    }

} // namespace Chandelier