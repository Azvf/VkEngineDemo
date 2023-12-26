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
# include_directories(${TINY_ENGINE_VENDOR_DIR}/stb)

# Vulkan
find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries (${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
    message(STATUS "Vulkan_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS}")
	message(STATUS "Vulkan_LIBRARIES: ${Vulkan_LIBRARIES}")
endif (Vulkan_FOUND)

# GLFW
set(GLFW_INCLUDE_DIRS "${TINY_ENGINE_VENDOR_DIR}/glfw/include")
set(GLFW_LIBRARIES "${TINY_ENGINE_VENDOR_DIR}/glfw/lib/glfw3.lib")
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} PRIVATE ${GLFW_LIBRARIES})
message(STATUS "GLFW_INCLUDE_DIRS: ${GLFW_INCLUDE_DIRS}")
message(STATUS "GLFW_LIBRARIES: ${GLFW_LIBRARIES}")