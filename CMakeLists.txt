cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

set(PROJECT_NAME VulkanTests)

project(${PROJECT_NAME})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Set some paths

set(SHADER_DIRECTORY ${CMAKE_SOURCE_DIR}/shader)

#Configure project to build with C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

# build glfw without docs/tests/examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

#Set source files
add_executable(${PROJECT_NAME}
        main.cpp
        VulkanTestApplication.h VulkanTestApplication.cpp)

#Add external dependencies
target_include_directories(${PROJECT_NAME} PUBLIC
    glfw/include
    ${Vulkan_INCLUDE_DIR}
    ${Vulkan_INCLUDE_DIRS}
    )

target_link_libraries(${PROJECT_NAME}
    glfw ${GLFW_LIBRARIES}
    ${Vulkan_LIBRARIES}
    )

IF (WIN32)
    # TODO: build shaders
ELSE()
    add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK
            COMMAND ${SHADER_DIRECTORY}/compile.sh
            WORKING_DIRECTORY ${SHADER_DIRECTORY})
ENDIF ()