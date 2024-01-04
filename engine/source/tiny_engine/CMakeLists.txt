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

# core base
file(GLOB TINY_ENGINE_CORE_BASE CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/core/base/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/base/*.cpp
)

# core meta
file(GLOB_RECURSE TINY_ENGINE_CORE_META CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/core/meta/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/meta/*.cpp
)

# _generated
file(GLOB TINY_ENGINE_GENERATED CONFIGURE_DEPENDS 
    ${TINY_ENGINE_SOURCES}/_generated/reflection/all_reflection.h
    ${TINY_ENGINE_SOURCES}/_generated/serializer/all_serializer.h
    ${TINY_ENGINE_SOURCES}/_generated/serializer/all_serializer.ipp
)

# headers(temp)
file(GLOB TINY_ENGINE_HEADERS CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

# sources(temp)
file(GLOB TINY_ENGINE_SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

# folder group
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${HEADER_FILES} 
    ${SOURCE_FILES}
    ${TINY_ENGINE_RENDER_SOURCES}
    ${TINY_ENGINE_UI_SOURCES}
    ${TINY_ENGINE_CORE_BASE}
    ${TINY_ENGINE_CORE_META}
    ${TINY_ENGINE_GENERATED}
)

add_executable(${PROJECT_NAME} 
    ${TINY_ENGINE_VENDORS}
    ${TINY_ENGINE_HEADERS}
    ${TINY_ENGINE_SOURCES}
    ${TINY_ENGINE_RENDER_SOURCES}
    ${TINY_ENGINE_UI_SOURCES}
    ${TINY_ENGINE_CORE_BASE}
    ${TINY_ENGINE_CORE_META}
    ${TINY_ENGINE_GENERATED}
)

# tiny engine include
include_directories(${TINY_ENGINE_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/render)

# vendor
include_directories(${TINY_ENGINE_VENDOR_DIR})

# glm
# include_directories(${TINY_ENGINE_VENDOR_DIR}/glm)

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