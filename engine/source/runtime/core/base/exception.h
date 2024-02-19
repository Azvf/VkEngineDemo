#pragma once

#include <string>
#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Chandelier {
    template<typename Result>
    class EngineException : public std::exception {
    public:
        EngineException(const std::string& err, const Result result)
            : m_err_string(err), m_result(result) {}

        virtual ~EngineException() throw() {}
        virtual const char* what() const throw() { return m_err_string.c_str(); }
        Result  GetErrorCode() const { return m_result; }
        const std::string& GetErrorString() const { return m_err_string; }
    
    public:
        static EngineException MakeEngineException(const std::string& err, const Result result,
            const std::string& func_name, const std::string& file_name, int line_no) {
            std::ostringstream err_log;
            err_log << func_name << " : " << err << " at " << file_name << ":" << line_no << std::endl;
            EngineException exception(err_log.str(), result);
            return exception;
        }
    
    private:
        std::string m_err_string;
        Result      m_result;
    };
}

enum EngineCode {
    Base_Engine_Code            = 2568,
    General_Assert_Code,
    Buffer_Size_Not_Match,
    Image_Layout_Not_Supported,
    None_Suitable_Mem_Type,
    Engine_Code_MAX_ENUM,
};

#define ENGINE_THROW_ERROR( err_str, result )                           \
    do {                                                                \
        assert(0);                                                      \
        throw EngineException<decltype(result)>::MakeEngineException(   \
            err_str, result, __FUNCTION__, __FILE__, __LINE__           \
        );                                                              \
    } while (0)


#define VULKAN_API_CALL( vulkan_api )                                       \
    do {                                                                    \
        VkResult result = vulkan_api;                                       \
        if( result != VK_SUCCESS ) {                                        \
            std::ostringstream err_log;                                     \
            err_log << #vulkan_api << " returned error " << result;         \
            throw EngineException<VkResult>::MakeEngineException(           \
                err_log.str(), result, __FUNCTION__, __FILE__, __LINE__     \
            );                                                              \
        }                                                                   \
        else                                                                \
        {                                                                   \
            /*std::cout << #vulkan_api << " returned error " << result << std::endl;*/ \
        }                                                                   \
    } while (0)

