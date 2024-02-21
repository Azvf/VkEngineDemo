file(GLOB tinyobjloader_sources CONFIGURE_DEPENDS  "${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader/*.h")
add_library(tinyobjloader INTERFACE ${tinyobjloader_sources})
target_include_directories(tinyobjloader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader)