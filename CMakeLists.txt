cmake_minimum_required(VERSION 3.12)
project(VkEngineDemo)
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/bin/$<CONFIG>>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/bin/$<CONFIG>>)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

add_subdirectory(src)
add_subdirectory(vk-bootstrap)
add_subdirectory(glm)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VkEngineDemo)
