cmake_minimum_required(VERSION 3.12)

project(VkEngineDemo)

# vendor
file(GLOB TINY_ENGINE_VENDORS CONFIGURE_DEPENDS 
    ${TINY_ENGINE_VENDOR_DIR}/stb/stb_image.h
    ${TINY_ENGINE_VENDOR_DIR}/tinyobjloader/tiny_obj_loader.h
)

# render
file(GLOB_RECURSE TINY_ENGINE_RENDER_SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/render/*.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/render/*.cpp
)

# resource
file(GLOB_RECURSE TINY_ENGINE_RESOURCE CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/resource/*.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/resource/*.cpp
)

# UI
file(GLOB_RECURSE TINY_ENGINE_UI_SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/UI/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UI/*.cpp
)

# runtime
file(GLOB_RECURSE TINY_ENGINE_RUNTIME CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/*.cpp
)

# _generated
file(GLOB TINY_ENGINE_GENERATED CONFIGURE_DEPENDS 
    ${TINY_ENGINE_SOURCES}/_generated/reflection/all_reflection.h
    ${TINY_ENGINE_SOURCES}/_generated/serializer/all_serializer.h
    ${TINY_ENGINE_SOURCES}/_generated/serializer/all_serializer.ipp
)

# vulkan
file(GLOB TINY_ENGINE_VULKAN CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.cpp
)

# editor
file(GLOB TINY_ENGINE_EDITOR CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/*.cpp
)

# folder group
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${TINY_ENGINE_GENERATED}
    ${TINY_ENGINE_RUNTIME}
    ${TINY_ENGINE_VULKAN} 
    ${TINY_ENGINE_EDITOR}
    ${TINY_ENGINE_RENDER_SOURCES}
    ${TINY_ENGINE_UI_SOURCES}
    ${TINY_ENGINE_RESOURCE}
)

add_executable(${PROJECT_NAME} 
    ${TINY_ENGINE_GENERATED}
    ${TINY_ENGINE_VENDORS}
    ${TINY_ENGINE_RUNTIME}
    ${TINY_ENGINE_VULKAN} 
    ${TINY_ENGINE_EDITOR}
    ${TINY_ENGINE_RENDER_SOURCES}
    ${TINY_ENGINE_UI_SOURCES}
    ${TINY_ENGINE_RESOURCE}
)

set(GLSLC_EXE ${ENGINE_DIR}/shaders/glslc.exe)
set(SHADERS ${ENGINE_DIR}/shaders)
add_custom_command(TARGET ${PROJECT_NAME}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo "Copying assets ${ENGINE_DIR}/assets to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ENGINE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets

                    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS}/generated
                    COMMAND ${CMAKE_COMMAND} -E echo "Running post-build script"
                    COMMAND ${CMAKE_COMMAND} -E echo "Compiling shaders: ${ENGINE_DIR}/shaders"
                    COMMAND ${GLSLC_EXE} ${SHADERS}/shader.vert                 -o ${SHADERS}/generated/base_vert.spv
                    COMMAND ${GLSLC_EXE} ${SHADERS}/shader.frag                 -o ${SHADERS}/generated/base_frag.spv
                    COMMAND ${GLSLC_EXE} ${SHADERS}/skybox.vert                 -o ${SHADERS}/generated/skybox_vert.spv
                    COMMAND ${GLSLC_EXE} ${SHADERS}/skybox.frag                 -o ${SHADERS}/generated/skybox_frag.spv
                    COMMAND ${GLSLC_EXE} ${SHADERS}/basic_P2T2.vert             -o ${SHADERS}/generated/brdf_lut_gen_vert.spv
                    COMMAND ${GLSLC_EXE} ${SHADERS}/brdf_lut_gen.frag           -o ${SHADERS}/generated/brdf_lut_gen_frag.spv
                    COMMAND ${GLSLC_EXE} ${SHADERS}/cubemap.vert                -o ${SHADERS}/generated/cubemap_prefilter_gen_vert.spv
                    COMMAND ${GLSLC_EXE} ${SHADERS}/cubemap_prefilter_gen.frag  -o ${SHADERS}/generated/cubemap_prefilter_gen_frag.spv
                    COMMAND ${GLSLC_EXE} ${SHADERS}/cubemap.vert                -o ${SHADERS}/generated/irradiance_convolution_vert.spv
                    COMMAND ${GLSLC_EXE} ${SHADERS}/irradiance_convolution.frag -o ${SHADERS}/generated/irradiance_convolution_frag.spv
                    COMMAND ${GLSLC_EXE} ${SHADERS}/shadowmap.vert              -o ${SHADERS}/generated/shadowmap_vert.spv
                    COMMAND ${CMAKE_COMMAND} -E echo "Compiling shaders executed successfully."
                    COMMAND ${CMAKE_COMMAND} -E echo "Copying shaders form ${SHADERS}/generated to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/shaders"
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADERS}/generated ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/shaders
                    )

target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX) 

# tiny engine include
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/editor)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vulkan)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/render)

# vendor
include_directories(${TINY_ENGINE_VENDOR_DIR})

# stb
target_link_libraries(${PROJECT_NAME} PRIVATE stb)

# imgui
include_directories(${TINY_ENGINE_VENDOR_DIR}/imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

# Vulkan
target_link_libraries (${PROJECT_NAME} PRIVATE ${Vulkan_Libraires})

# glfw
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_Libraries})

# json11
target_link_libraries(${PROJECT_NAME} PRIVATE json11)

# lemon
target_link_libraries(${PROJECT_NAME} PRIVATE ${Lemon_Libraries})

# spirv_cross
target_link_libraries(${PROJECT_NAME} PRIVATE ${Spirv_Cross_Libraries})

### precompile
# set global var used by precompile
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
set(PICCOLO_RUNTIME_HEADS "${HEADER_FILES}" PARENT_SCOPE)
file(GLOB_RECURSE EDITOR_HEADER_FILES CONFIGURE_DEPENDS "include/*.h")
set(PICCOLO_EDITOR_HEADS "${EDITOR_HEADER_FILES}" PARENT_SCOPE)