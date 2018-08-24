cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

set(PROJECT_NAME VulkanTests)

project(${PROJECT_NAME})

# Make sure executable is in build root directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# add findVulkan if old VERSION

if(${CMAKE_VERSION} VERSION_LESS "3.7.0")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
endif()

set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH}:/usr/local/opt/llvm/lib)

add_subdirectory(src)

#add_custom_target(GetAssets ALL
#        DEPENDS meshes/spot.obj textures/spot.png meshes/chalet.obj textures/chalet.jpg)
#add_custom_command(OUTPUT meshes/chalet.obj textures/chalet.jpg meshes/spot.obj
#        COMMAND ${CMAKE_SOURCE_DIR}/getassets.sh
#        )
#add_dependencies(${PROJECT_NAME} GetAssets)

#add_custom_target(CopyShaders ALL)
#add_dependencies(${PROJECT_NAME} CopyShaders)
#add_custom_command(TARGET CopyShaders POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders)
