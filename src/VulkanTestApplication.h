//
// Created by va61jine on 03.08.18.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <iostream>
#include <limits>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/hash.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include <chrono>

#include <array>
#include "shader-loader/ShaderUpdater.h"


struct QueueFamilyIndices {
    uint32_t graphicsFamily = std::numeric_limits<uint32_t>::max();
    uint32_t presentFamily = std::numeric_limits<uint32_t>::max();

    bool isComplete() {
        return graphicsFamily != std::numeric_limits<uint32_t>::max() && presentFamily != std::numeric_limits<uint32_t>::max();
    }
};

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct Vertex {
    glm::vec4 pos;
    glm::vec4 normal;
    glm::vec4 color;
    glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription = {};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[0].offset = static_cast<uint32_t>(offsetof(Vertex, pos));

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[1].offset = static_cast<uint32_t>(offsetof(Vertex, normal));

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[2].offset = static_cast<uint32_t>(offsetof(Vertex, color));

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[3].offset = static_cast<uint32_t>(offsetof(Vertex, texCoord));

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && normal == other.normal && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec4>()(vertex.pos) ^
                    (hash<glm::vec4>()(vertex.normal))<< 1 ^
                    (hash<glm::vec4>()(vertex.color) << 2)) >> 1) ^
                    (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}


struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 spherize;
    glm::mat4 normal;
};

struct LightBufferObject {
    glm::vec4 lightPos;
    glm::vec4 ambientColor;
};

class VulkanTestApplication {
public:
    void run();
    void requestFramebufferResize();
    void requestShaderReload();
private:
    bool m_framebufferResized = false;
    bool m_reloadShaders = false;
    const std::string applicationName= "VulkanTest";
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    const std::string MODEL_PATH = "meshes/spot.obj";
    const std::string TEXTURE_PATH = "textures/spot.png";
    const int MAX_FRAMES_IN_FLIGHT = 2;
    const uint64_t WAIT_TIME = 1000000000;

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

    //Shader compiler
    ShaderUpdater updater;

    // Window to render to
    GLFWwindow* window;
    // Instance of Vulkan (only one should exist I think)
    vk::Instance instance;
    // Debug callback
    VkDebugReportCallbackEXT callback;
    // Physical device (GPU)
    vk::PhysicalDevice physicalDevice;
    // Logical device (there can be multiple logical devices for one physical one)
    vk::Device device;
    // Queue for graphics
    vk::Queue graphicsQueue;
    // Surface to render to this is an extension
    vk::SurfaceKHR surface;
    // Queue for presenting images
    vk::Queue presentQueue;
    // Swapchain with multiple images that allow for buffering
    vk::SwapchainKHR swapChain;
    // Handles for the images of the swap chain
    std::vector<vk::Image> swapChainImages;
    // Formate and size of swap chain images
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::ImageView> swapChainImageViews;

    vk::RenderPass renderPass;

    // pipeline is used in a renderpass
    vk::Pipeline graphicsPipeline;

    vk::DescriptorSetLayout descriptorSetLayout;

    // layout of the pipeline (number of uniforms and push values)
    vk::PipelineLayout pipelineLayout;

    std::vector<vk::Framebuffer> swapChainFramebuffers;

    VkCommandPool commandPool;

    std::vector<vk::CommandBuffer> commandBuffers;

    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    size_t currentFrame = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vk::Buffer modelBuffer;
    vk::DeviceMemory modelBufferMemory;

    vk::DeviceSize vertexOffset;
    vk::DeviceSize indexOffset;

    vk::DeviceSize uniformStartOffset;
    vk::DeviceSize uniformMemorySize;
    vk::DeviceSize lightStartOffset;
    vk::DeviceSize lightMemorySize;

    vk::Buffer uniformBuffer;
    vk::DeviceMemory uniformBufferMemory;

    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;

    uint32_t mipLevels;
    vk::Image textureImage;
    vk::DeviceMemory textureImageMemory;

    vk::ImageView textureImageView;
    vk::Sampler textureSampler;

    vk::Image depthImage;
    vk::DeviceMemory depthImageMemory;
    vk::ImageView depthImageView;

    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

    vk::Image colorImage;
    vk::DeviceMemory colorImageMemory;
    vk::ImageView colorImageView;

    void createModelBuffer();

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
    void createInstance();

    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void setupDebugCallback();

    void pickPhysicalDevice();

    bool isDeviceSuitable(const vk::PhysicalDevice &pT);

    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

    void createLogicalDevice();

    void createSurface();

    bool checkDeviceExtensionSupport(const vk::PhysicalDevice &device);

    uint32_t swapChainImageCount;

    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

    void createSwapChain();

    void createImageViews();

    void createRenderPass();

    void createGraphicsPipeline();

    vk::ShaderModule createShaderModule(const std::vector<uint32_t> &code);

    void createFramebuffers();

    void createCommandPool();

    void createCommandBuffers();

    void drawFrame();

    void createSyncObjects();

    void recreateSwapChain();

    void cleanupSwapChain();

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                      vk::Buffer &buffer, vk::DeviceMemory &bufferMemory, vk::SharingMode sharingMode = vk::SharingMode::eExclusive);

    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

    void createDescriptorSetLayout();

    void createUniformBuffer();

    void updateUniformBuffer(uint32_t imageIndex);

    void createDescriptorSets();

    void createDescriptorPool();

    void createTextureImage();


    void createImage(int width, int height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format,
                     vk::ImageTiling tiling, vk::ImageUsageFlags bits, vk::MemoryPropertyFlags flagBits,
                     vk::Image &image,
                     vk::DeviceMemory &imageMemory);

    vk::CommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

    void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout, uint32_t mipLevels);

    void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

    void createTextureImageView();

    vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags,
                                  uint32_t mipLevels);

    void createTextureSampler();

    void createDepthResources();

    vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                                   vk::FormatFeatureFlags features);
    vk::Format findDepthFormat();
    bool hasStencilComponent(vk::Format format);

    void loadModel();

    void generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight,
                         uint32_t mipLevels);

    vk::SampleCountFlagBits getMaxUsableSampleCount();

    void createColorResources() ;

    void reloadShaders();

    void getSwapChainImageCount();

    void buildSwapChain();

    bool initShaders();

    void showPhysicalDeviceStats();
    vk::PhysicalDeviceLimits physicalDeviceLimits;
    void getPhysicalDeviceLimits();

    void testBufferAlignment();
};
