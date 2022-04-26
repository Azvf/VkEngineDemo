cmake_minimum_required(VERSION 3.12)

project(VkEngineDemo)

include_directories(${CMAKE_SOURCE_DIR}/vk-bootstrap/src)

add_executable(${PROJECT_NAME} main.cpp ../stb/stb_image.h)

message(STATUS "vkb path: ${CMAKE_SOURCE_DIR}")

# Vulkan
find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries (${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
    message(STATUS "Vulkan_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS}")
	message(STATUS "Vulkan_LIBRARIES: ${Vulkan_LIBRARIES}")
endif (Vulkan_FOUND)

# GLFW
set(GLFW_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/libs/include")
set(GLFW_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/lib/glfw3.lib")
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} PRIVATE ${GLFW_LIBRARIES})
message(STATUS "GLFW_INCLUDE_DIRS: ${GLFW_INCLUDE_DIRS}")
message(STATUS "GLFW_LIBRARIES: ${GLFW_LIBRARIES}")

# vk-bootstrap
link_libraries(${PROJECT_NAME} vk-bootstrap::vk-bootstrap)


