
# Set some paths

set(SHADER_DIRECTORY ${CMAKE_SOURCE_DIR}/src/shaders)

#Configure project to build with C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

message("${GLM_INCLUDE_DIRS}")

# build glfw without docs/tests/examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

#Set source files
add_executable(${PROJECT_NAME}
        main.cpp
        VulkanTestApplication.h VulkanTestApplication.cpp stb_image.h)

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
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK
            COMMAND ${SHADER_DIRECTORY}/compile.sh
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)
ENDIF ()

# copy textures to output

set(TEXTURE_DIR ${CMAKE_SOURCE_DIR}/textures)

file(GLOB files "${TEXTURE_DIR}/*.jpg")
foreach(file ${files})
    get_filename_component(FILENAME ${file} NAME)
    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${file}
            ${CMAKE_BINARY_DIR}/textures/${FILENAME})
endforeach()