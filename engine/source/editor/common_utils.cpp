#include "common_utils.h"

#include <cassert>
#include <fstream>

#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace Chandelier
{
    std::filesystem::path GetExeDirPath()
    {
#ifdef _WIN32
        wchar_t buffer[1024];
        if (GetModuleFileNameW(NULL, buffer, sizeof(buffer)))
        {
            return std::filesystem::path(buffer).parent_path();
        }
        else
        {
            assert(0);
            return std::filesystem::path();
        }
#else
        assert(0);
        return std::filesystem::path();
#endif
    }

    std::vector<char> readBinaryFile(const char* filepath)
    {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);

        if (!file.is_open())
        {
            std::string msg = "Failed to open file " + std::string(filepath) + "!";
            throw "error";
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (file.read(buffer.data(), size))
        {
            return buffer;
        }
        else
        {
            std::string msg = "Failed to read file " + std::string(filepath) + "!";
            throw "error";
        }
    }
} // namespace Utils