cmake_minimum_required(VERSION 3.12)

project(VkEngineDemo)

add_executable(${PROJECT_NAME} 
    RenderCfg.h
    VkContext.h
    VkContext.cpp
    VkUtil.h
    VkUtil.cpp
    Buffer.h
    Buffer.cpp
    Image.h
    Image.cpp
    SwapChain.h
    SwapChain.cpp
    Mesh.h
    Mesh.cpp
    Texture.h
    Texture.cpp
    RenderResources.h
    RenderResources.cpp
    Pipeline.h
    Pipeline.cpp
    Renderer.h
    Renderer.cpp
    Window.h
    Window.cpp
    Descriptor.h
    Descriptor.cpp
    Sampler.h
    Sampler.cpp
    Uniform.h
    Uniform.cpp
    Engine.h
    Engine.cpp
    VkEngineDemo.cpp
    ../ThirdParty/stb/stb_image.h 
    ../ThirdParty/tinyobjloader/tiny_obj_loader.h
)

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

# glm
include_directories(${CMAKE_SOURCE_DIR}/glm)


