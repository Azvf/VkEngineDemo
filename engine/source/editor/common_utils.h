#pragma once

#include <filesystem>
#include <vector>

namespace Chandelier
{
    std::filesystem::path GetExeDirPath();
    std::vector<char>     readBinaryFile(const char* filepath);

} // namespace Utils