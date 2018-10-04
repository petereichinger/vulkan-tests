//
// Created by va61jine on 03.08.18.
//

#include "VulkanTestApplication.h"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <set>
#include <fstream>
#include <unordered_map>
#include "file-utils/FileHelpers.h"
#include "helpers/pow2.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glm/gtx/string_cast.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace tinyobj {
    bool operator<(tinyobj::index_t const& lhs,
                   tinyobj::index_t const& rhs) {
        if (lhs.vertex_index == rhs.vertex_index) {
            if (lhs.normal_index == rhs.normal_index) {
                return lhs.texcoord_index < rhs.texcoord_index;
            } else
                return lhs.normal_index < rhs.normal_index;
        } else
            return lhs.vertex_index < rhs.vertex_index;
    }

    bool operator==(tinyobj::index_t const& lhs,
                    tinyobj::index_t const& rhs) {
        if (lhs.vertex_index == rhs.vertex_index) {
            if (lhs.normal_index == rhs.normal_index) {
                return lhs.texcoord_index == rhs.texcoord_index;
            } else
                return false;
        } else
            return false;
    }
};

void generate_normals(tinyobj::attrib_t& attribs, tinyobj::mesh_t& mesh) {
    std::vector<uint32_t> indices;
    std::map<tinyobj::index_t, uint32_t> vertexToIndexMap;
    // save vertex positions as glm vectors
    std::vector<glm::fvec3> positions(mesh.indices.size());
    uint32_t index = 0;
    for (auto const& vert_properties : mesh.indices) {
        auto it = vertexToIndexMap.find(vert_properties);
        if (it == vertexToIndexMap.end()) {
            indices.push_back(index);
        }
        else {
            indices.push_back(it->second);
        }
        positions[index] = glm::fvec3{attribs.vertices[3 * vert_properties.vertex_index + 0]
                ,attribs.vertices[3 * vert_properties.vertex_index + 1]
                ,attribs.vertices[3 * vert_properties.vertex_index + 2]
        };
        ++index;
    }
    // calculate triangle normal
    std::vector<glm::fvec3> normals(mesh.num_face_vertices.size(), glm::fvec3{0.0f});
    // size_t index_offset = 0;
    for (unsigned i = 0; i < mesh.num_face_vertices.size(); i+=3) {
        assert(mesh.num_face_vertices[i] == 3);
        glm::fvec3 normal = glm::cross(positions[i+1] - positions[i], positions[i+2] - positions[i]);
        // accumulate vertex normals
        normals[i] += normal;
        normals[i+1] += normal;
        normals[i+2] += normal;
    }

    for (unsigned i = 0; i < mesh.indices.size(); ++i) {
        tinyobj::index_t& vert_properties = mesh.indices[i];
        // not a duplicate
        auto unique_idx = indices.at(i);
        if (unique_idx == i) {
            vert_properties.normal_index = uint32_t(attribs.normals.size() / 3);
            glm::fvec3 normal = glm::normalize(normals[i]);
            attribs.normals.emplace_back(normal[0]);
            attribs.normals.emplace_back(normal[1]);
            attribs.normals.emplace_back(normal[2]);
        }
        else {
            vert_properties.normal_index = mesh.indices.at(unique_idx).normal_index;
        }
    }
}

vk::Result CreateDebugReportCallback(vk::Instance instance, vk::Device device,
                                     const VkDebugReportCallbackCreateInfoEXT &createInfo,
                                     VkDebugReportCallbackEXT *callback) {
    vk::DispatchLoaderDynamic dildy(instance, device);
    return vk::Result(dildy.vkCreateDebugReportCallbackEXT(instance, &createInfo, nullptr, callback));
}

void DestroyDebugReportCallback(vk::Instance instance, vk::Device device, VkDebugReportCallbackEXT callback) {
    vk::DispatchLoaderDynamic dildy(instance, device);

    dildy.vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
}
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t obj,
        size_t location,
        int32_t code,
        const char *layerPrefix,
        const char *msg,
        void *userData) {

    std::cerr << "validation layer: " << msg << std::endl;

    return VK_FALSE;
}



static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanTestApplication*>(glfwGetWindowUserPointer(window));
    app->requestFramebufferResize();
}

static void quitCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_RELEASE) {
        return;
    }
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_R) {
        auto app = reinterpret_cast<VulkanTestApplication*>(glfwGetWindowUserPointer(window));
        app->requestShaderReload();
    }
}



void VulkanTestApplication::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void VulkanTestApplication::requestFramebufferResize() {
    m_framebufferResized = true;
}

void VulkanTestApplication::requestShaderReload() {
    m_reloadShaders = true;
}

void VulkanTestApplication::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, quitCallback);
}

void VulkanTestApplication::initVulkan() {
    createInstance();
    setupDebugCallback();
    createSurface();
    pickPhysicalDevice();
    getPhysicalDeviceLimits();
    showMemoryStats();

    createLogicalDevice();

    getSwapChainImageCount();

    createDescriptorSetLayout();

    createCommandPool();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    loadModel();
//
//    createVertexBuffer();
//    createIndexBuffer();

    createModelBuffer();

    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSets();
    createSyncObjects();

    if (!initShaders()) {
        throw std::runtime_error("Error while compiling shaders initially, exiting.");
    }

    buildSwapChain();
}



void VulkanTestApplication::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo = {};
    appInfo.pApplicationName = "Vulkan Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    vk::InstanceCreateInfo createInfo = {};
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    instance = vk::createInstance(createInfo);
}

void VulkanTestApplication::mainLoop() {

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (updater.checkForModifications()) {
//            vertCode = updater.getShaderCode("shaders/triangle.vert");
//            fragCode = updater.getShaderCode("shaders/triangle.frag");
            m_reloadShaders = true;
        }

        if (m_reloadShaders) {
            reloadShaders();
        }

        drawFrame();
    }

    device.waitIdle();
}

std::vector<const char *> VulkanTestApplication::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // Check if Vulkan supports all extensions required by GLFW
    std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

    for (auto i = 0; i < extensions.size(); ++i) {
        auto found = std::find_if(supportedExtensions.begin(), supportedExtensions.end(),
                                  [&](VkExtensionProperties props) {
                                      return strcmp(props.extensionName, extensions[i]);
                                  });
        if (found == supportedExtensions.end()) {
            std::stringstream ss;
            ss << "extension '" << extensions[i] << "' not supported";
            throw std::runtime_error(ss.str());
        }
    }

    return extensions;
}

bool VulkanTestApplication::checkValidationLayerSupport() {
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    for (const char *layerName : validationLayers) {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}


void VulkanTestApplication::cleanup() {
    cleanupSwapChain();

    device.destroy(textureSampler);
    device.destroy(textureImageView);

    device.destroy(textureImage);
    device.free(textureImageMemory);

    device.destroy(descriptorPool);

    device.destroy(descriptorSetLayout);

//    for (size_t i = 0; i < swapChainImageCount; i++) {
//        device.destroy( uniformBuffers[i]);
//        device.free(uniformBuffersMemory[i]);
//        device.destroy(lightBuffers[i]);
//        device.free(lightBuffersMemory[i]);
//    }

    device.destroy(uniformBuffer);
    device.free(uniformBufferMemory);

    device.destroy(modelBuffer);
    device.free(modelBufferMemory);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        device.destroy(renderFinishedSemaphores[i]);
        device.destroy(imageAvailableSemaphores[i]);
        device.destroy(inFlightFences[i]);
    }

    device.destroy(commandPool);

    if (enableValidationLayers) {
        DestroyDebugReportCallback(instance, device, callback);
    }

    device.destroy();

    instance.destroy(surface);
    instance.destroy();

    glfwDestroyWindow(window);

    glfwTerminate();
}



void VulkanTestApplication::setupDebugCallback() {
    if (!enableValidationLayers) return;

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;

    if (CreateDebugReportCallback(instance, device, createInfo, &callback) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to set up debug callback!");
    }
}

void VulkanTestApplication::pickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

    for (const auto &device : devices) {
        std::cout << "Found GPU: " << device.getProperties().deviceName << std::endl;
        if (isDeviceSuitable(device) && !physicalDevice) {
            physicalDevice = device;
            msaaSamples = getMaxUsableSampleCount();
        }
    }

    if (!physicalDevice) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    std::cout << "Using " << physicalDevice.getProperties().deviceName <<std::endl;
}

bool VulkanTestApplication::isDeviceSuitable(const vk::PhysicalDevice &device) {
    // Can query properties and features of the physical device

    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();
    vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
    std::cout << "Memory Allocs: " << deviceProperties.limits.maxMemoryAllocationCount << std::endl;

    return deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && indices.isComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;
}

QueueFamilyIndices VulkanTestApplication::findQueueFamilies(vk::PhysicalDevice device) {
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    uint32_t i = 0U;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {
            indices.graphicsFamily = i;
        }

        auto presentSupport = device.getSurfaceSupportKHR(i, surface);

        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }


    return indices;
}

void VulkanTestApplication::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;
    for (auto queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    vk::DeviceCreateInfo createInfo = {};

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    device = physicalDevice.createDevice(createInfo);

    graphicsQueue = device.getQueue(indices.graphicsFamily,0);
    presentQueue = device.getQueue(indices.presentFamily, 0);
}


void VulkanTestApplication::createSurface() {
    VkSurfaceKHR cSurface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &cSurface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    surface = cSurface;
}

bool VulkanTestApplication::checkDeviceExtensionSupport(const vk::PhysicalDevice &device) {
    std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanTestApplication::querySwapChainSupport(vk::PhysicalDevice device) {
    SwapChainSupportDetails details;

    details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
    details.formats = device.getSurfaceFormatsKHR(surface);
    details.presentModes = device.getSurfacePresentModesKHR(surface);

    return details;
}

vk::SurfaceFormatKHR
VulkanTestApplication::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
        return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
    }
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

vk::PresentModeKHR
VulkanTestApplication::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
    vk::PresentModeKHR bestMode = vk::PresentModeKHR::eImmediate;

    for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        } else if (availablePresentMode == vk::PresentModeKHR::eFifo) {
            bestMode = availablePresentMode;
        }
    }

    return bestMode;
}

vk::Extent2D VulkanTestApplication::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        vk::Extent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width,
                                      std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
                                       std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

void VulkanTestApplication::getSwapChainImageCount() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    swapChainImageCount = imageCount;
}

void VulkanTestApplication::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    getSwapChainImageCount();

    vk::SwapchainCreateInfoKHR createInfo = {};
    createInfo.surface = surface;

    createInfo.minImageCount = swapChainImageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {(uint32_t) indices.graphicsFamily, (uint32_t) indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = nullptr;

    swapChain = device.createSwapchainKHR(createInfo);

    swapChainImages = device.getSwapchainImagesKHR(swapChain);
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void VulkanTestApplication::createImageViews() {
    swapChainImageViews.resize(swapChainImageCount);

    for (uint32_t i = 0; i < swapChainImageCount; i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor, 1);
    }
}

void VulkanTestApplication::createGraphicsPipeline() {

    vk::ShaderModule vertShaderModule = createShaderModule(updater.getShaderCode("shaders/triangle.vert"));
    vk::ShaderModule fragShaderModule = createShaderModule(updater.getShaderCode("shaders/triangle.frag"));

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::Viewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor = {};
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = swapChainExtent;

    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = msaaSamples;

    vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
//    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.colorWriteMask =
                    vk::ColorComponentFlagBits::eR |
                    vk::ColorComponentFlagBits::eG |
                    vk::ColorComponentFlagBits::eB |
                    vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

    vk::GraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;

    graphicsPipeline = device.createGraphicsPipeline(nullptr, pipelineInfo);

    device.destroy(fragShaderModule);
    device.destroy(vertShaderModule);
}

vk::ShaderModule VulkanTestApplication::createShaderModule(const std::vector<uint32_t> &code) {
    vk::ShaderModuleCreateInfo createInfo = {};
    createInfo.codeSize = code.size() * 4;
    createInfo.pCode = code.data();
    return device.createShaderModule(createInfo);
}

void VulkanTestApplication::createRenderPass() {
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = msaaSamples;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout =vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentDescription depthAttachment = {};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentDescription colorAttachmentResolve = {};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
    colorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachmentResolve.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachmentResolve.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentResolveRef = {};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    vk::SubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits ::eColorAttachmentOutput;
    dependency.srcAccessMask = vk::AccessFlags();
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

    std::array<vk::AttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    renderPass = device.createRenderPass(renderPassInfo);
}

void VulkanTestApplication::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<vk::ImageView, 3> attachments = {
                colorImageView,
                depthImageView,
                swapChainImageViews[i]
        };

        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        swapChainFramebuffers[i] = device.createFramebuffer(framebufferInfo);
    }
}

void VulkanTestApplication::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    commandPool = device.createCommandPool(poolInfo);
}

void VulkanTestApplication::createCommandBuffers() {
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = (uint32_t) swapChainFramebuffers.size();

    commandBuffers = device.allocateCommandBuffers(allocInfo);

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        
        vk::CommandBufferBeginInfo beginInfo = {};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;

        auto commandBuffer = commandBuffers[i];

        commandBuffer.begin(beginInfo);

        vk::RenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = vk::Offset2D {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<vk::ClearValue, 2> clearValues = {
                vk::ClearColorValue {std::array<float,4>{0.0f, 0.0f, 0.0f, 1.0f}},
                vk::ClearDepthStencilValue {1.0f, 0}};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();


        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

        std::array<vk::Buffer,1> vertexBuffers {modelBuffer};
        std::array<vk::DeviceSize,1> offsets = {vertexOffset};
        commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(modelBuffer, indexOffset, vk::IndexType::eUint32);
//        std::array<vk::Buffer,1> vertexBuffers {vertexBuffer};
//        std::array<vk::DeviceSize,1> offsets = {0};
//        commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
//        commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);


        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSets[i], nullptr);
        commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        commandBuffer.endRenderPass();

        commandBuffer.end();
    }
}

void VulkanTestApplication::drawFrame() {
    device.waitForFences(inFlightFences[currentFrame], VK_TRUE, WAIT_TIME);

    uint32_t imageIndex;
    try {
        vk::ResultValue<uint32_t> resultValue = device.acquireNextImageKHR(swapChain, WAIT_TIME, imageAvailableSemaphores[currentFrame], nullptr);
        imageIndex = resultValue.value;
    } catch (vk::OutOfDateKHRError& error){
        recreateSwapChain();
        return;
    }

    if (imageIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("invalid image index");
    }


    updateUniformBuffer(imageIndex);

    vk::SubmitInfo submitInfo = {};

    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    device.resetFences(inFlightFences[currentFrame]);

    graphicsQueue.submit(submitInfo,inFlightFences[currentFrame]);

    vk::PresentInfoKHR presentInfo = {};

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    bool recreate = m_framebufferResized;
    try {
        auto result = presentQueue.presentKHR(presentInfo);
        if (result == vk::Result::eSuboptimalKHR) {
            recreate = true;
        }
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present swap chain image: " + vk::to_string(result));
        }
    } catch (vk::OutOfDateKHRError&) {
        recreate = true;
    }

    if (recreate) {
        m_framebufferResized = false;
        recreateSwapChain();
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanTestApplication::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo = {};

    vk::FenceCreateInfo fenceInfo = {};
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        inFlightFences[i] = device.createFence(fenceInfo);
        imageAvailableSemaphores[i] = device.createSemaphore(semaphoreInfo);
        renderFinishedSemaphores[i] = device.createSemaphore(semaphoreInfo);
    }
}
uint32_t VulkanTestApplication::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}


void VulkanTestApplication::createModelBuffer() {
    vk::DeviceSize vertexSize = sizeof(Vertex) * vertices.size();
    vk::DeviceSize indexSize = sizeof(uint32_t) * indices.size();

    vertexOffset = 0;
    indexOffset = vertexOffset + vertexSize;


    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    createBuffer(vertexSize + indexSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer, stagingBufferMemory);

    void* data = device.mapMemory(stagingBufferMemory, 0, vertexSize);
    std::memcpy(data, vertices.data(), (size_t) vertexSize);
    device.unmapMemory(stagingBufferMemory);

    data = device.mapMemory(stagingBufferMemory,vertexSize, indexSize);
    std::memcpy(data, indices.data(), (size_t) indexSize);
    device.unmapMemory(stagingBufferMemory);

    createBuffer(vertexSize + indexSize,
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer| vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal, modelBuffer, modelBufferMemory);


    copyBuffer(stagingBuffer, modelBuffer, vertexSize + indexSize);

    device.destroy(stagingBuffer);
    device.free(stagingBufferMemory);
}


void VulkanTestApplication::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::BufferCopy copyRegion = {};
    copyRegion.size = size;
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

    endSingleTimeCommands(commandBuffer);
}


void VulkanTestApplication::cleanupSwapChain() {
    device.destroy(colorImageView);
    device.destroy(colorImage);
    device.free(colorImageMemory);

    device.destroy(depthImageView);
    device.destroy(depthImage);
    device.free(depthImageMemory);


    for (auto swapChainFramebuffer : swapChainFramebuffers) {
        device.destroy(swapChainFramebuffer);
    }

    device.freeCommandBuffers(commandPool, commandBuffers);

    device.destroy(graphicsPipeline);
    device.destroy(pipelineLayout);
    device.destroy(renderPass);

    for (auto swapChainImageView : swapChainImageViews) {
        device.destroy(swapChainImageView);
    }

    device.destroy(swapChain);
}
void VulkanTestApplication::recreateSwapChain() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    device.waitIdle();

    cleanupSwapChain();

    buildSwapChain();
}

void VulkanTestApplication::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                         vk::MemoryPropertyFlags properties, vk::Buffer &buffer,
                                         vk::DeviceMemory &bufferMemory, vk::SharingMode sharingMode) {
    vk::BufferCreateInfo bufferInfo = {};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;
    buffer = device.createBuffer(bufferInfo);

    vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);

//    std::cout << vk::to_string(memRequirements.memoryTypeBits) <<std::endl;
    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    bufferMemory = device.allocateMemory(allocInfo);

    device.bindBufferMemory(buffer,bufferMemory, 0);
}



void VulkanTestApplication::createDescriptorSetLayout() {
    // Descriptor set layout is used in the pipeline
    vk::DescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutBinding lightLayoutBinding = {};
    lightLayoutBinding.binding = 1;
    lightLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    lightLayoutBinding.descriptorCount = 1;
    lightLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 2;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    std::array<vk::DescriptorSetLayoutBinding, 3> bindings = {uboLayoutBinding,lightLayoutBinding, samplerLayoutBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
}

void VulkanTestApplication::createUniformBuffer() {

    auto minUboSize = physicalDeviceLimits.minUniformBufferOffsetAlignment;
    uniformMemorySize = nextPowerOfTwo(std::max(minUboSize, (unsigned long long)sizeof(UniformBufferObject)));
    lightMemorySize = nextPowerOfTwo(std::max(minUboSize, (unsigned long long)sizeof(LightBufferObject)));
    std::cout << minUboSize << " " << uniformMemorySize << " " << lightMemorySize << std::endl;
    uniformStartOffset = 0;
    lightStartOffset = uniformStartOffset + uniformMemorySize * swapChainImageCount;
    auto totalUBOSize = swapChainImageCount * (lightMemorySize + uniformMemorySize);

    createBuffer(totalUBOSize, vk::BufferUsageFlagBits::eUniformBuffer,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 uniformBuffer, uniformBufferMemory);
}

void VulkanTestApplication::updateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo = {};
    auto rotate = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo.model = glm::rotate(rotate, time * glm::radians(45.0f), glm::vec3(0,1,0));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(90.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    ubo.spherize = glm::vec4(0.5f * (1.0f + glm::sin(glm::pi<float>() * time)));
    ubo.normal = glm::inverseTranspose(ubo.view * ubo.model);

    LightBufferObject lbo = {};
    lbo.lightPos = glm::rotate(glm::mat4(1.0f), time * glm::radians(180.0f), glm::vec3(0.0f,1.0f, 0.0f)) * glm::vec4(10,10,0,1);
//    lbo.lightPos = glm::vec4(-10, 10, 10, 1);
    lbo.ambientColor = glm::vec4(0.1f);

    void* data = device.mapMemory(uniformBufferMemory, uniformStartOffset + uniformMemorySize * currentImage, uniformMemorySize);
    memcpy(data, &ubo, sizeof(ubo));
    device.unmapMemory(uniformBufferMemory);

    data = device.mapMemory(uniformBufferMemory, lightStartOffset + lightMemorySize * currentImage, lightMemorySize);
    memcpy(data, &lbo, sizeof(lbo));
    device.unmapMemory(uniformBufferMemory);
}

void VulkanTestApplication::createDescriptorPool() {
    getSwapChainImageCount();
    std::array<vk::DescriptorPoolSize, 3> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImageCount);
    poolSizes[1].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImageCount);
    poolSizes[2].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(swapChainImageCount);

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImageCount);

    descriptorPool = device.createDescriptorPool(poolInfo);
}

void VulkanTestApplication::createDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts(swapChainImageCount, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImageCount);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapChainImageCount);
    descriptorSets = device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < swapChainImageCount; i++) {
        vk::DescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffer;
        bufferInfo.offset = uniformStartOffset + i * uniformMemorySize;
        bufferInfo.range = uniformMemorySize;

        vk::DescriptorBufferInfo lightInfo = {};
        lightInfo.buffer = uniformBuffer;
        lightInfo.offset = lightStartOffset + i * lightMemorySize;
        lightInfo.range = lightMemorySize;

        vk::DescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        std::array<vk::WriteDescriptorSet, 3> descriptorWrites = {};

        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightInfo;

        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &imageInfo;

        device.updateDescriptorSets(descriptorWrites, nullptr);
    }
}

void VulkanTestApplication::createTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    auto imageSize = static_cast<vk::DeviceSize>(texWidth * texHeight * 4);

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer, stagingBufferMemory);

    void *data = device.mapMemory(stagingBufferMemory,0,imageSize);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    device.unmapMemory(stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(texWidth, texHeight, mipLevels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Unorm,
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                textureImage, textureImageMemory);

    transitionImageLayout(textureImage,vk::Format::eR8G8B8A8Unorm,
                          vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eTransferDstOptimal, mipLevels);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    generateMipmaps(textureImage, vk::Format::eR8G8B8A8Unorm, texWidth, texHeight, mipLevels);

    device.destroy(stagingBuffer);
    device.free(stagingBufferMemory);
}

void
VulkanTestApplication::createImage(int width, int height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples,
                                   vk::Format format,
                                   vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                   vk::MemoryPropertyFlags properties, vk::Image &image,
                                   vk::DeviceMemory &imageMemory) {
    vk::ImageCreateInfo imageInfo = {};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;


    image = device.createImage(imageInfo);

    vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    imageMemory = device.allocateMemory(allocInfo);

    device.bindImageMemory(image,imageMemory, 0);
}

vk::CommandBuffer VulkanTestApplication::beginSingleTimeCommands() {
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void VulkanTestApplication::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
    commandBuffer.end();

    vk::SubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    graphicsQueue.submit(submitInfo, nullptr);
    graphicsQueue.waitIdle();

    device.freeCommandBuffers(commandPool, commandBuffer);
}

void VulkanTestApplication::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                                                  vk::ImageLayout newLayout, uint32_t mipLevels) {
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier = {};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = vk::AccessFlags();
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlags();
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlags();
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage
        , vk::DependencyFlags(), nullptr, nullptr, barrier);

    endSingleTimeCommands(commandBuffer);
}

void VulkanTestApplication::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::BufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = vk::Offset3D {0, 0, 0};
    region.imageExtent = vk::Extent3D {
            width,
            height,
            1
    };

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);

    endSingleTimeCommands(commandBuffer);
}

void VulkanTestApplication::createTextureImageView() {
    textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Unorm,vk::ImageAspectFlagBits::eColor, mipLevels);
}

vk::ImageView VulkanTestApplication::createImageView(vk::Image image, vk::Format format,
                                                     vk::ImageAspectFlags aspectFlags,
                                                     uint32_t mipLevels) {
    vk::ImageViewCreateInfo viewInfo = {};
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    return device.createImageView(viewInfo);
}

void VulkanTestApplication::createTextureSampler() {
    vk::SamplerCreateInfo samplerInfo = {};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;

    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;

    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels);

    textureSampler = device.createSampler(samplerInfo);
}

void VulkanTestApplication::createDepthResources() {
    vk::Format depthFormat = findDepthFormat();

    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat,
            vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
            vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);

    transitionImageLayout(depthImage, depthFormat, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);
}

vk::Format VulkanTestApplication::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                                                      vk::FormatFeatureFlags features) {
    for (vk::Format format : candidates) {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

vk::Format VulkanTestApplication::findDepthFormat() {
    return findSupportedFormat(
            {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

bool VulkanTestApplication::hasStencilComponent(vk::Format format) {
    return format ==  vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void VulkanTestApplication::loadModel() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, MODEL_PATH.c_str())) {
        throw std::runtime_error(err);
    }

//    generate_normals(attrib,shapes[0].mesh);



    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};


    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};
            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                    1.0f
            };

            if (attrib.normals.size() > 0) {
                vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                        0.0f
                };
            } else {
                std::cerr << "No normals in mesh" << std::endl;
            }

            vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {0.5f, 1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

}

void VulkanTestApplication::generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth,
                                            int32_t texHeight,
                                            uint32_t mipLevels) {
    // Check if image format supports linear blitting
    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(imageFormat);

    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits ::eSampledImageFilterLinear)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier = {};
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,vk::PipelineStageFlagBits::eTransfer,vk::DependencyFlags(),
                                      nullptr,nullptr,barrier);


        vk::ImageBlit blit = {};
        blit.srcOffsets[0] = vk::Offset3D {0, 0, 0};
        blit.srcOffsets[1] = vk::Offset3D {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = vk::Offset3D {0, 0, 0};
        blit.dstOffsets[1] = vk::Offset3D { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        commandBuffer.blitImage(
                image, vk::ImageLayout::eTransferSrcOptimal,
                image, vk::ImageLayout::eTransferDstOptimal,
                blit,vk::Filter::eLinear);

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                vk::DependencyFlags(), nullptr,nullptr, barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;


    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                  vk::DependencyFlags(), nullptr,nullptr, barrier);

    endSingleTimeCommands(commandBuffer);
}

vk::SampleCountFlagBits VulkanTestApplication::getMaxUsableSampleCount() {
    vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();

    vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
    if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
    if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
    if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
    if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
    if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }

    return vk::SampleCountFlagBits::e1;
}

void VulkanTestApplication::createColorResources() {
    vk::Format colorFormat = swapChainImageFormat;

    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, colorImage, colorImageMemory);
    colorImageView = createImageView(colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);

    transitionImageLayout(colorImage, colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
}

void VulkanTestApplication::reloadShaders() {
    graphicsQueue.waitIdle();

    device.destroy(pipelineLayout);
    device.destroy(graphicsPipeline);
    device.freeCommandBuffers(commandPool, commandBuffers);

    createGraphicsPipeline();
    createCommandBuffers();

    m_reloadShaders = false;
}

void VulkanTestApplication::buildSwapChain() {
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createCommandBuffers();
}

bool VulkanTestApplication::initShaders() {
    bool success=  updater.registerShader("shaders/triangle.frag");
    success |= updater.registerShader("shaders/triangle.vert");

    return success;
}

void VulkanTestApplication::showMemoryStats() {
    vk::PhysicalDeviceMemoryProperties memProps = physicalDevice.getMemoryProperties();

    std::cout << "HEAPS" << std::endl;
    for (int i = 0; i < memProps.memoryHeapCount; ++i) {
        std::cout <<i << ": "<< memProps.memoryHeaps[i].size <<" " << vk::to_string(memProps.memoryHeaps[i].flags) << std::endl;
    }
    
    std:: cout << "MEMORY" <<std::endl;

    for (int j = 0; j < memProps.memoryTypeCount; ++j) {
        auto memoryType = memProps.memoryTypes[j];
        std::cout << j << ": " << memoryType.heapIndex << " " << vk::to_string(memoryType.propertyFlags) << std::endl;
    }
}

void VulkanTestApplication::getPhysicalDeviceLimits() {
    physicalDeviceLimits = physicalDevice.getProperties().limits;
}
