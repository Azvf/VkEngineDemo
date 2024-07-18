#pragma once

#include <filesystem>
#include <vector>

namespace Chandelier
{
    namespace fs = std::filesystem;

    #define EXPAND_STR(s) INNER_EXPAND_STR(s)
    #define INNER_EXPAND_STR(s) #s

    #define SINGLETON(CLASS_NAME) class CLASS_NAME : public Singleton<CLASS_NAME> 
    template<class T>
    class Singleton
    {
    public:
        static T& GetInstance()
        {
            static T instance;
            return instance;
        }

        Singleton(const Singleton&) = delete;
        Singleton(Singleton&&)      = delete;

        Singleton& operator=(const Singleton&) = delete;
        Singleton& operator=(Singleton&&)      = delete;

    protected:
        Singleton() = default;
    };

    std::filesystem::path GetExeDirPath();
    std::vector<char>     readBinaryFile(const char* filepath);

} // namespace Utils