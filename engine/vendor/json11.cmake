add_library(json11 STATIC ${CMAKE_CURRENT_SOURCE_DIR}/json11/json11.cpp)
target_include_directories(json11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json11)