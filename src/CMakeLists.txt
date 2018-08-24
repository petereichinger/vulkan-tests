
# Set some paths

set(SHADER_DIRECTORY ${CMAKE_SOURCE_DIR}/src/shaders)

#Configure project to build with C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

# build glfw without docs/tests/examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

add_subdirectory(glslang)

#Set source files
add_executable(${PROJECT_NAME} file-utils/FileWatcher.cpp file-utils/FileWatcher.h shader-loader/ShaderUpdater.cpp shader-loader/ShaderUpdater.h)

#set_target_properties(${PROJECT_NAME} PROPERTIES
#        CMAKE_CXX_STANDARD 17
#        CMAKE_CXX_STANDARD_REQUIRED ON
#        CMAKE_CXX_EXTENSIONS OFF
#        )

target_sources(${PROJECT_NAME} PUBLIC main.cpp
        VulkanTestApplication.h VulkanTestApplication.cpp
        file-utils/FileHelpers.h file-utils/FileHelpers.cpp
        shader-loader/ShaderLoader.cpp shader-loader/ShaderLoader.h)

#Add external dependencies
target_include_directories(${PROJECT_NAME} PUBLIC
    glfw/include
    glm
    external
    ${Vulkan_INCLUDE_DIR}
    ${Vulkan_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src
    )
target_link_libraries(${PROJECT_NAME}
    glfw ${GLFW_LIBRARIES}
    ${Vulkan_LIBRARIES}
        glslang SPIRV
    )
