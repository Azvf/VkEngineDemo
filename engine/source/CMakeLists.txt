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

# runtime core base
file(GLOB TINY_ENGINE_RUNTIME_CORE_BASE CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/core/base/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/core/base/*.cpp
)

# runtime core meta
file(GLOB_RECURSE TINY_ENGINE_RUNTIME_CORE_META CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/core/meta/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/core/meta/*.cpp
)

# runtime core math
file(GLOB_RECURSE TINY_ENGINE_RUNTIME_CORE_MATH CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/core/math/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/core/math/*.cpp
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
    ${TINY_ENGINE_VULKAN} 
    ${TINY_ENGINE_EDITOR}
    ${TINY_ENGINE_RENDER_SOURCES}
    ${TINY_ENGINE_UI_SOURCES}
    ${TINY_ENGINE_RUNTIME_CORE_BASE}
    ${TINY_ENGINE_RUNTIME_CORE_META}
    ${TINY_ENGINE_RUNTIME_CORE_MATH}
    ${TINY_ENGINE_GENERATED}
)

add_executable(${PROJECT_NAME} 
    ${TINY_ENGINE_VENDORS}
    ${TINY_ENGINE_VULKAN} 
    ${TINY_ENGINE_EDITOR}
    ${TINY_ENGINE_RENDER_SOURCES}
    ${TINY_ENGINE_UI_SOURCES}
    ${TINY_ENGINE_RUNTIME_CORE_BASE}
    ${TINY_ENGINE_RUNTIME_CORE_META}
    ${TINY_ENGINE_RUNTIME_CORE_MATH}
    ${TINY_ENGINE_GENERATED}
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

### precompile
# set global var used by precompile
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
set(PICCOLO_RUNTIME_HEADS "${HEADER_FILES}" PARENT_SCOPE)
file(GLOB_RECURSE EDITOR_HEADER_FILES CONFIGURE_DEPENDS "include/*.h")
set(PICCOLO_EDITOR_HEADS "${EDITOR_HEADER_FILES}" PARENT_SCOPE)