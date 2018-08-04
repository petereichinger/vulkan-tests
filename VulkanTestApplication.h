//
// Created by va61jine on 03.08.18.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <iostream>
#include <limits>

struct QueueFamilyIndices {
    uint32_t graphicsFamily = std::numeric_limits<uint32_t>::max();
    uint32_t presentFamily = std::numeric_limits<uint32_t>::max();

    bool isComplete() {
        return graphicsFamily != std::numeric_limits<uint32_t>::max() && presentFamily != std::numeric_limits<uint32_t>::max();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanTestApplication {
public:
    void run();

private:
    const std::string applicationName= "VulkanTest";
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const std::vector<const char*> validationLayers = {
            "VK_LAYER_LUNARG_standard_validation"
    };

    const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


    // Window to render to
    GLFWwindow* window;
    // Instance of Vulkan (only one should exist I think)
    VkInstance instance;
    // Debug callback
    VkDebugReportCallbackEXT callback;
    // Physical device (GPU)
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    // Logical device (there can be multiple logical devices for one physical one)
    VkDevice device;
    // Queue for graphics
    VkQueue graphicsQueue;
    // Surface to render to this is an extension
    VkSurfaceKHR surface;
    // Queue for presenting images
    VkQueue presentQueue;
    // Swapchain with multiple images that allow for buffering
    VkSwapchainKHR swapChain;
    // Handles for the images of the swap chain
    std::vector<VkImage> swapChainImages;
    // Formate and size of swap chain images
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;


    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
    void createInstance();

    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void setupDebugCallback();

    void pickPhysicalDevice();

    bool isDeviceSuitable(VkPhysicalDevice const &pT);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void createLogicalDevice();

    void createSurface();

    bool checkDeviceExtensionSupport(VkPhysicalDevice const &device);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const &  availableFormats);
    VkPresentModeKHR chooseSwapPresentMode( std::vector<VkPresentModeKHR> const & availablePresentModes);
    VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR const & capabilities);

    void createSwapChain();

    void createImageViews();

    void createGraphicsPipeline();
};