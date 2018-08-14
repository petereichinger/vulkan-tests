cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

set(PROJECT_NAME VulkanTests)

project(${PROJECT_NAME})

# Make sure executable is in build root directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# add findVulkan if old VERSION

if(${CMAKE_VERSION} VERSION_LESS "3.7.0")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
endif()

#message("${CMAKE_BINARY_DIR}")

add_subdirectory(src)

