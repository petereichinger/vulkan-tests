//
// Created by va61jine on 03.08.18.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <iostream>


struct QueueFamilyIndices {
    int graphicsFamily = -1;

    bool isComplete() {
        return graphicsFamily >= 0;
    }
};

class VulkanTestApplication {
public:
    void run();

private:
    const std::string applicationName= "VulkanTest";
    const int WIDTH = 800;
    const int HEIGHT = 600;

    const std::vector<const char*> validationLayers = {
            "VK_LAYER_LUNARG_standard_validation"
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
    void createInstance();

    GLFWwindow* window;
    VkInstance instance;
    VkDebugReportCallbackEXT callback;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;


    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void setupDebugCallback();

    void pickPhysicalDevice();

    bool isDeviceSuitable(VkPhysicalDevice const &pT);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void createLogicalDevice();
};