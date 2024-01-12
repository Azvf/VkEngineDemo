# engine root directory alias
set(ENGINE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/MP")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VkEngineDemo)
endif()

# glfw
set(GLFW_Includes ${TINY_ENGINE_VENDOR_DIR}/glfw/include)
include_directories(${GLFW_Includes})
set(GLFW_Libraries ${TINY_ENGINE_VENDOR_DIR}/glfw/lib/glfw3.lib)

# vulkan
set(Vulkan_Includes ${TINY_ENGINE_VENDOR_DIR}/VulkanSDK/include)
include_directories(${Vulkan_Includes})
if(WIN32)
    set(Vulkan_Libraires ${TINY_ENGINE_VENDOR_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
    # set(glslangValidator_executable ${TINY_ENGINE_VENDOR_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe)
    # add_compile_definitions("PICCOLO_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32")
else()
     message(FATAL_ERROR "Unknown Platform")
endif()

add_subdirectory(vendor)
add_subdirectory(source/parser)
add_subdirectory(source)

#codegen
set(CODEGEN_TARGET "PiccoloPreCompile")
include(precompile/precompile.cmake)

add_dependencies(VkEngineDemo "${CODEGEN_TARGET}")
add_dependencies("${CODEGEN_TARGET}" MetaParser)
