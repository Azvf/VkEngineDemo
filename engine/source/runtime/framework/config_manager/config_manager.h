#pragma once

#include <filesystem>

namespace Chandelier
{
    class ConfigManager
    {
    public:
        void Initialize(const std::filesystem::path& config_file_path);
    };
} // namespace Chandelier
