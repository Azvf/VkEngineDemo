# engine root directory alias
set(ENGINE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/MP")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VkEngineDemo)
endif()

# todo: add local vulkan library
# set(vulkan_include ${THIRD_PARTY_DIR}/VulkanSDK/include)

add_subdirectory(source/parser)
add_subdirectory(source/tiny_engine)
add_subdirectory(vendor/glm)
# add_subdirectory(vendor/msdf-atlas-gen)

set(CODEGEN_TARGET "PiccoloPreCompile")
include(precompile/precompile.cmake)
set_target_properties("${CODEGEN_TARGET}" PROPERTIES FOLDER "Engine" )

add_dependencies(VkEngineDemo "${CODEGEN_TARGET}")
add_dependencies("${CODEGEN_TARGET}" MetaParser)
