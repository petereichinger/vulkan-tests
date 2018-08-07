cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

set(PROJECT_NAME VulkanTests)

project(${PROJECT_NAME})

# Make sure executable is in build root directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)