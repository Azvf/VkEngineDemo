cmake_minimum_required(VERSION 3.12)

project(VkEngineDemo)

file(GLOB TINY_ENGINE_HEADERS CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

file(GLOB TINY_ENGINE_SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

file(GLOB TINY_ENGINE_VENDORS CONFIGURE_DEPENDS 
    ${TINY_ENGINE_VENDOR_DIR}/stb/stb_image.h
    ${TINY_ENGINE_VENDOR_DIR}/tinyobjloader/tiny_obj_loader.h
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}" 
    FILES ${TINY_ENGINE_HEADERS} ${TINY_ENGINE_SOURCES}
)

add_executable(${PROJECT_NAME} 
    ${TINY_ENGINE_HEADERS}
    ${TINY_ENGINE_SOURCES}
    ${TINY_ENGINE_VENDORS}
)

# tiny engine include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

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

#glfw
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_Libraries})
