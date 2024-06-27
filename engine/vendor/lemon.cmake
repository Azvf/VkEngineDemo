file(GLOB lemon_files CONFIGURE_DEPENDS  "${CMAKE_CURRENT_SOURCE_DIR}/lemon/*.h")
add_library(lemon INTERFACE ${lemon_files})
target_include_directories(lemon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lemon)