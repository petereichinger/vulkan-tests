#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include "VulkanTestApplication.h"
#include "ShaderCompiler.h"
int main() {
    VulkanTestApplication app;
    ShaderCompiler sc;

    try {
        sc.loadShader("shaders/triangle.vert");
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
