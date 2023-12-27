set(VENDOR_FOLDER "Vendor")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(NOT TARGET imgui)
    include(imgui.cmake)
    set_target_properties(imgui PROPERTIES FOLDER ${VENDOR_FOLDER}/imgui)
endif()

if(NOT TARGET stb)
    include(stb.cmake)
    set_target_properties(stb PROPERTIES FOLDER ${VENDOR_FOLDER}/stb)
endif()

# if(NOT TARGET tinyobjloader)
#     add_subdirectory(tinyobjloader)
#     set_target_properties(tinyobjloader PROPERTIES FOLDER ${VENDOR_FOLDER}/tinyobjloader)
#     set_target_properties(uninstall PROPERTIES FOLDER ${VENDOR_FOLDER}/tinyobjloader)
# endif()

# if(NOT TARGET glfw)
#     option(GLFW_BUILD_EXAMPLES "" OFF)
#     option(GLFW_BUILD_TESTS "" OFF)
#     option(GLFW_BUILD_DOCS "" OFF)
#     option(GLFW_INSTALL "" OFF)
#     add_subdirectory(glfw)
#     set_target_properties(glfw PROPERTIES FOLDER ${third_party_folder}/glfw)
#     set_target_properties(update_mappings PROPERTIES FOLDER ${third_party_folder}/glfw)
# endif()
