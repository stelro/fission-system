/* ========================================================================
   $File: vulkan_base.cc $
   $Date: Fri Apr 26 21:22:44 2019 $
   $Revision: $
   $Creator: Ro Stelmach $
   $Notice: (C) Copyright 2019 by Ro Orestis Stelmach. All Rights Reserved. $
   ======================================================================== */

#include "renderer/vulkan_base.hh"
#include "core/fission.hh"
#include "core/settings.hh"
#include "math/math_utils.hh"
#include "math/matrix_transformations.hh"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <set>

namespace fn {

// @brief Proxy function that handles
// the load of VkCreateDebugUtilsMessengerEXT function
// because this function it is not loaded automatically
  VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  void DestroyDebugUtilsMessengerEXT(VkInstance instnace,
                                     VkDebugUtilsMessengerEXT debugMessenger,
                                     const VkAllocationCallbacks *pAllocator) {

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instnace, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
      func(instnace, debugMessenger, pAllocator);
    }
  }

// TODO: move this function to seperate file
  static std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      log::fatal("Faild to open file: %s\n", filename.c_str());
    }

    auto fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }

  VulkanBase::VulkanBase(std::shared_ptr<Settings> settings) noexcept
    : m_window(nullptr), m_settings(settings) {
#ifdef NDEBUG
    m_enableValidationLayers = false;
#else
    m_enableValidationLayers = true;
#endif
  }

  VulkanBase::~VulkanBase() noexcept {
    // Empty Destructor
  }

  void VulkanBase::run() noexcept {

    initWindow();
    initVulkan();
    mainLoop();
    cleanUp();
  }

  void VulkanBase::initWindow() noexcept {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window =
      glfwCreateWindow(static_cast<int>(m_settings->getWidth()),
                       static_cast<int>(m_settings->getHeight()),
                       m_settings->getEngineName().c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, frameBufferResizedCallback);
  }

  void VulkanBase::initVulkan() noexcept {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
  }

  void VulkanBase::mainLoop() noexcept {

    while (!glfwWindowShouldClose(m_window)) {
      glfwPollEvents();

      // Drawing
      drawFrame();
    }

    // Wait the logical device to finish operations before exiting mainloop
    vkDeviceWaitIdle(m_device);
  }

  void VulkanBase::cleanUp() noexcept {

    cleanupSwapChain();

    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

    vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
    vkFreeMemory(m_device, m_indexBufferMemory, nullptr);

    vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(m_device, m_semaphores.renderHasFinished[i], nullptr);
      vkDestroySemaphore(m_device, m_semaphores.imageIsAvailable[i], nullptr);
      vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(m_device, m_commandPool, nullptr);

    vkDestroyDevice(m_device, nullptr);

    if (m_enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }

  void VulkanBase::createInstance() noexcept {

    if (m_enableValidationLayers && !checkValidationLayerSupport()) {
      log::fatal("Validation layers are requested, but they are not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_settings->getEngineName().c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (m_enableValidationLayers) {
      createInfo.enabledLayerCount =
        static_cast<uint32_t>(m_validationLayers.size());
      createInfo.ppEnabledLayerNames = m_validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &m_instance));
  }

  bool VulkanBase::checkValidationLayerSupport() const noexcept {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : m_validationLayers) {

      if (std::any_of(availableLayers.cbegin(), availableLayers.cend(),
                      [&](VkLayerProperties prop) {
                        return (std::strcmp(layerName, prop.layerName) == 0);
                      }))
        return (true);
    }

    return (false);
  }

  std::vector<const char *> VulkanBase::getRequiredExtensions() const noexcept {
    uint32_t glfwExtensionsCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionsCount);

    if (m_enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }

  void VulkanBase::setupDebugMessenger() noexcept {
    if (!m_enableValidationLayers)
      return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;

    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr,
                                     &m_debugMessenger) != VK_SUCCESS) {
      log::error("Failed to set up debug messenger!");
    }
  }

  void VulkanBase::pickPhysicalDevice() noexcept {

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      log::fatal("Faild to find GPUs with Vulkan Support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    for (const auto &device : devices) {
      if (isDeviceSuitable(device)) {
        m_physicalDevice = device;
        break;
      }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
      log::fatal("Failed to find a suitable GPU!");
    }

    /// Get information about the physical device
    VkPhysicalDeviceProperties deviceProperites = {};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperites);

    log::info("Picked physical device: %s\n", deviceProperites.deviceName);
  }

  bool VulkanBase::isDeviceSuitable(VkPhysicalDevice device) const noexcept {

    QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionSupported = checkDeviceextensionsupport(device);

    bool swapChainAdequate = false;
    if (extensionSupported) {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
      swapChainAdequate = !swapChainSupport.formats.empty() &&
        !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionSupported && swapChainAdequate;
  }

  QueueFamilyIndices VulkanBase::findQueueFamilies(VkPhysicalDevice device) const
    noexcept {

    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
      if (queueFamily.queueCount > 0 &&
          queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

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

  void VulkanBase::createLogicalDevice() noexcept {

    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {

      VkDeviceQueueCreateInfo queueCreateInfo = {};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount =
      static_cast<uint32_t>(m_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

    if (m_enableValidationLayers) {
      createInfo.enabledLayerCount =
        static_cast<uint32_t>(m_validationLayers.size());
      createInfo.ppEnabledLayerNames = m_validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    VK_CHECK_RESULT(
      vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));
    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0,
                     &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
  }

  void VulkanBase::createSurface() noexcept {
    VK_CHECK_RESULT(
      glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface));
  }

  bool VulkanBase::checkDeviceextensionsupport(VkPhysicalDevice device) const
    noexcept {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(m_deviceExtensions.begin(),
                                             m_deviceExtensions.end());

    for (const auto &extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  SwapChainSupportDetails
  VulkanBase::querySwapChainSupport(VkPhysicalDevice device) const noexcept {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface,
                                              &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount,
                                         nullptr);

    if (formatCount != 0) {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount,
                                           details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface,
                                              &presentModeCount, nullptr);

    if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, m_surface, &presentModeCount, details.presentModes.data());
    }

    return (details);
  }

  VkSurfaceFormatKHR VulkanBase::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) const noexcept {

    if (availableFormats.size() == 1 &&
        availableFormats[0].format == VK_FORMAT_UNDEFINED) {
      // if the surface has no preferred format
      return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    // if we are not free to choose any format, then we'll go through the list
    // and see if the prefered format is available:
    auto iterator = std::find_if(
      availableFormats.cbegin(), availableFormats.cend(),
      [](VkSurfaceFormatKHR format) {
        return ((format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
                (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR));
      });

    if (iterator != std::end(availableFormats)) {
      return *iterator;
    } else {
      // if all of the above will fail, we can just settle with
      // the first available format

      // @idea, we could start ranking the available formats based on how good
      // they are?
      return availableFormats[0];
    }
  }

  VkPresentModeKHR VulkanBase::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) const noexcept {

    /**
       There are four possible modes available in vulkan:

       VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by our applicaiton
       are transfered to the screen right away, with may result in tearing!!!

       VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue, where the display
       takes an image from the fron of the queue when the display is refreshed
       and the program inserts rendered images at the back of the queue. If the
       queue is full then the program has to wait. This is the most similar to
       vertical sync as found in modern games. the moment that the display is
       refresed is known as vertical blank.

       VK_PRESENT_MODE_FIF_RELAXED_KHR: This mode only differs from the previous
       one if the application is late and the queue was empty at the last vertical
       blank. Instead of waiting for the next vertical blank, the image
       is transfered right away when it finally arives. This may result in visible
       tearing.

       VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode.
       Instead of blocking the application when the queue is full, the images that
       are already queued are simply replaced with the newer ones. This mode can
       be used to implement triple buffering, which allows you to avoid tearing
       with significantly less latency issues that standard vertical sync, that
       used double buffering.
    */

    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto &availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
      } else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        bestMode = availablePresentMode;
      }
    }

    return bestMode;
  }

  VkExtent2D
  VulkanBase::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const
    noexcept {

    // The range of possible resolutions is defined in the
    // VkSurfaceCapabilitiesKHR structure.

    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    } else {
      int width, height;
      glfwGetFramebufferSize(m_window, &width, &height);

      VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                                 static_cast<uint32_t>(height)};

      actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
      actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);
      return actualExtent;
    }
  }

  void VulkanBase::createSwapChain() noexcept {
    auto swapChainSupport = querySwapChainSupport(m_physicalDevice);

    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    auto extent = chooseSwapExtent(swapChainSupport.capabilities);

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;

    // How many images we would like to have in the swap chain?
    // sometimes we have to wait on the driver to complete internal
    // operations before we can acquire another image to render to.
    // Therefore is recomended to request at least one more image
    // than the minimum.
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = findQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                     indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;     // optional
      createInfo.pQueueFamilyIndices = nullptr; // optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped =
      VK_TRUE; // We don't care about the color of pixels that are obscured

    // It is possible that the current swap chain will become invaled or
    // unoptimized, e.g becase of w WINDOW RESIZE,.
    // In that case, the swap chain needs to be recreated from scratch and the
    // refrence to the old one must be specified in the field below.
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create the actuall swap chain
    VK_CHECK_RESULT(
      vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain));

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount,
                            m_swapChainImages.data());
  }

  void VulkanBase::createImageViews() noexcept {
    m_swapChainImagesViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
      VkImageViewCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = m_swapChainImages[i];
      // The viewtype field specify how the image should be treated.
      // for example it allows us to treate images as 1D textures or
      // 2D textures or 3D textures and cube maps
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = m_swapChainImageFormat;

      // The components field allows us to swizzle the color channles around
      // for example, we can map all of the channels to the red channel for a
      // monochrome texture.
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      // The subresourceRange field describes what the image's purpose is
      // and which part of the image should be accessed
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      VK_CHECK_RESULT(vkCreateImageView(m_device, &createInfo, nullptr,
                                        &m_swapChainImagesViews[i]));
    }
  }

  void VulkanBase::createGraphicsPipeline() noexcept {
    // @stel -> find a better way to handle this ?
    // maybe pass a factory object to load shaders at compile time?
    auto vertShaderCode = readFile("../shaders/texture.vert.spv");
    auto fragShaderCode = readFile("../shaders/texture.frag.spv");

    auto vertexShaderModule = createShaderModule(vertShaderCode);
    auto fragmentShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShaderModule;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragmentShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo,
                                                      fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributesDescription = Vertex::getAttributesDescriptions();

    /// The VkPipelineVertexInputStateCreateInfo describes the format of the
    /// vertex data that will be passed to the vertex shader.
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributesDescription.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributesDescription.data();

    /// the VkPipelineInputAssemblyStateCreateInfo struct describles 2 things:
    /// first what kind of geometry will be drawn from the verticies and if
    /// primatives restart should be enabled.
    /**
       VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from verticies
       VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reouse
       VK_PRIMITIVE_TOPOLOGY_LINE_STRIN: the end vertex of every line is used as
       start vertx for the next line VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle
       from every 3 verticies without reouse VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
       the second and third vertex of every triangle are used as first two
       vertices of the next triangle
    */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    /// A viewport basically describes the region of the framebuffer that the
    /// output will be rendered to!
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChainExtent.width);
    viewport.height = static_cast<float>(m_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    /// Rasterizer
    /// the rasterizer takes the geometry that is shaped by the vertices from the
    /// vertex shader and turns it into fragments to be colored bu the fragment
    /// shader. it also performs depth testing and face culling and the scissor
    /// test.
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    /// The polygonMode determines how fragments are generated for geometry.
    /**
       The following modes are available:
       VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
       VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
       VK_POLYGON_MODE_POINT: polygon vertices are drawn as points

       ******
       Using any mode other that fill requires enabling a GPU special feature:
       rasterizer.lineWidth = 1.0f;
       *****
       */
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optonal

    /// the VkPipelineMultisamplingStateCreateInfo struct configures
    /// multi-sampling, which is one of the ways to perform anti-aliasig.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr,
                                           &m_pipelineLayout));

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1,
                                              &pipelineInfo, nullptr,
                                              &m_graphicsPipeline));

    vkDestroyShaderModule(m_device, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertexShaderModule, nullptr);
  }

  VkShaderModule
  VulkanBase::createShaderModule(const std::vector<char> &code) const noexcept {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    VK_CHECK_RESULT(
      vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule));

    return shaderModule;
  }

  void VulkanBase::createRenderPass() noexcept {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // The loadOp and storeOp determine what to do with the data
    // in the attachment before rendering and after rendering
    // We choose to clear the framebuffer to black before rendering
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK_RESULT(
      vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
  }

  void VulkanBase::createFrameBuffers() noexcept {

    m_swapChainFrameBuffers.resize(m_swapChainImagesViews.size());

    // Iterate over the image views and crate the framebuffers from them
    for (size_t i = 0; i < m_swapChainImagesViews.size(); i++) {
      VkImageView attachments[] = {m_swapChainImagesViews[i]};

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = m_renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = m_swapChainExtent.width;
      framebufferInfo.height = m_swapChainExtent.height;
      // Our swap chaing images are simgle images
      // so the number of layers are one!
      framebufferInfo.layers = 1;

      VK_CHECK_RESULT(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr,
                                          &m_swapChainFrameBuffers[i]));
    }
  }

  void VulkanBase::createCommandPool() noexcept {

    // @Research -> it is worth to cache queuefamilies?
    // we are using the call over 4 times in this class
    auto queueFamilyIndices = findQueueFamilies(m_physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // We are going to to record commands for drawing, which is why we are
    // choosing the graphics queue.
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0; // Optional

    VK_CHECK_RESULT(
      vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
  }

  void VulkanBase::createCommandBuffers() noexcept {
    m_commandBuffers.resize(m_swapChainFrameBuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    VK_CHECK_RESULT(
      vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()));

    // Starting command buffer recording
    for (size_t i = 0; i < m_commandBuffers.size(); i++) {
      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
      beginInfo.pInheritanceInfo = nullptr; // Optional

      VK_CHECK_RESULT(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo));

      // Starting a render pass

      VkRenderPassBeginInfo renderPassInfo = {};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass = m_renderPass;
      renderPassInfo.framebuffer = m_swapChainFrameBuffers[i];

      // Define the size of the render area
      renderPassInfo.renderArea.offset = {0, 0};
      renderPassInfo.renderArea.extent = m_swapChainExtent;

      // Clear color is simply black with 100% opacity
      VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
      renderPassInfo.clearValueCount = 1;
      renderPassInfo.pClearValues = &clearColor;

      // Begin to record commands
      vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo,
                           VK_SUBPASS_CONTENTS_INLINE);

      // Bind the graphics pipeline
      vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_graphicsPipeline);

      // Bind the vertex buffer (into the vertex shader) during rendering
      // operations
      VkBuffer vertexBuffers[] = {m_vertexBuffer};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer, 0,
                           VK_INDEX_TYPE_UINT16);

      vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);


      // Draw a triangle
      /**
         The paramters is as following:
         vertexCount: vertex count, 3 for a triangle
         instanceCount: used for instanced rendering, 1 if we are not doing that
         firestVertex: used as an offset int the vertex buffer, defines the lowest
         value of gl_VertexInex firstInstance: used as an offset for instance
         rendering
      */
      // vkCmdDraw(m_commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1,
      // 0,
      //           0);

      vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(indices.size()),
                       1, 0, 0, 0);

      // The render pass now can be ended
      vkCmdEndRenderPass(m_commandBuffers[i]);

      VK_CHECK_RESULT(vkEndCommandBuffer(m_commandBuffers[i]));
    }
  }

  void VulkanBase::drawFrame() noexcept {

    // The drawFrame() function perform the following operations:
    // - Acquire an image from the swap chain
    // - Execute the command buffer with that image as attachment in the
    // framebuffer
    // - Return the image to the swapchain for presentation

    // index of the swapchain image that has become available.
    // the index refer to to the VkImage in oure swapchainimages array. We are
    // going to use that index to pick the right command buffer

    // Wait for the frame to be finished
    vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE,
                    std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(
      m_device, m_swapChain, std::numeric_limits<uint64_t>::max(),
      m_semaphores.imageIsAvailable[m_currentFrame], VK_NULL_HANDLE,
      &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      log::fatal("Failed to aquire swap chain image");
    }

    updateuniformbuffers(imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
      m_semaphores.imageIsAvailable[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

    // Which semaphores to signal once the command buffers have
    // finished the execution
    VkSemaphore signalSemaphores[] = {
      m_semaphores.renderHasFinished[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

    VK_CHECK_RESULT(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo,
                                  m_inFlightFences[m_currentFrame]));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_frameBufferHasResized) {
      m_frameBufferHasResized = false;
      recreateSwapChain();
    } else if (result != VK_SUCCESS) {
      log::fatal("failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  void VulkanBase::createSyncObjects() noexcept {
    m_semaphores.imageIsAvailable.resize(MAX_FRAMES_IN_FLIGHT);
    m_semaphores.renderHasFinished.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr,
                                        &m_semaphores.imageIsAvailable[i]));
      VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr,
                                        &m_semaphores.renderHasFinished[i]));
      VK_CHECK_RESULT(
        vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]));
    }
  }

  void VulkanBase::recreateSwapChain() noexcept {

    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(m_window, &width, &height);
      glfwWaitEvents();
    }

    // We call vkDeviceWaitIdle beacuse we shouldn't touch resources
    // that are still in use.
    vkDeviceWaitIdle(m_device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
  }

  void VulkanBase::cleanupSwapChain() noexcept {
    for (auto framebuffer : m_swapChainFrameBuffers) {
      vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(m_device, m_commandPool,
                         static_cast<uint32_t>(m_commandBuffers.size()),
                         m_commandBuffers.data());

    vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);

    for (auto imageView : m_swapChainImagesViews) {
      vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
      vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
      vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
  }

  void VulkanBase::createVertexBuffer() noexcept {

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    // We are using a staging buffer to use jost visible buffer as termporary
    // buffer and use a device local buffer as actual vertex buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    // memcpy(destination, source, byets); YES I easily forget memcpy prototype
    memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer,
                 m_vertexBufferMemory);

    copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
  }

  uint32_t VulkanBase::findMemoryType(uint32_t typeFilter,
                                      VkMemoryPropertyFlags properties) const
    noexcept {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
        return i;
      }
    }

    FN_ASSERT_M(false, "Failed to find suitable memory type!");
  }

  void VulkanBase::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkBuffer &buffer,
                                VkDeviceMemory &bufferMemory) noexcept {

    /// @fix -> maybe the buffer creation must be moved to it's own
    /// file and to have it's own implementation

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

    VK_CHECK_RESULT(
      vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory));

    // If the allocation was successfull, then we can associate this memory with
    // the buffer using:
    vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
  }

  void VulkanBase::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                              VkDeviceSize size) noexcept {

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    // Start Recording the command buffer
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    // Execute the command buffer to complete the transfer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
  }

  void VulkanBase::createIndexBuffer() noexcept {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

    copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
  }

  void VulkanBase::createDescriptorSetLayout() noexcept {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    // The stageflags can be also a combination of VkShaderStageFlagsBits
    // or it can be a VK_SHADER_STAGE_ALL_GRAPHICS
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr,
                                                &m_descriptorSetLayout));
  }

  void VulkanBase::createUniformBuffers() noexcept {

    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(m_swapChainImages.size());
    m_uniformBuffersMemory.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
      createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   m_uniformBuffers[i], m_uniformBuffersMemory[i]);
    }
  }

  void VulkanBase::updateuniformbuffers(uint32_t currentimage) noexcept {
    static auto startTime = std::chrono::high_resolution_clock::now();


    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
      currentTime - startTime)
      .count();



    UniformBufferObject ubo = {};
    ubo.model = Math::rotate(Matrix4(1.0f), Vec3(0.0f, 0.0f, 1.0f),
                             time * Math::radians(90.f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0, 0.0f, 0.0f),
                           glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(Math::radians(45.0f), m_swapChainExtent.width / float(m_swapChainExtent.height), 0.1f, 10.0f);

    ubo.proj[1][1] *= -1;

    void *data;
    vkMapMemory(m_device, m_uniformBuffersMemory[currentimage], 0, sizeof(ubo), 0, &data);
    memcpy(data,&ubo, sizeof(ubo));
    vkUnmapMemory(m_device, m_uniformBuffersMemory[currentimage]);

    // We left here
  }

  void VulkanBase::createDescriptorPool() noexcept {
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    poolInfo.maxSets = static_cast<uint32_t>(m_swapChainImages.size());

    VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool));
  }

  void VulkanBase::createDescriptorSets() noexcept {
    std::vector<VkDescriptorSetLayout> layouts(m_swapChainImages.size(), m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(m_swapChainImages.size());
    VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()));

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = m_uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      VkWriteDescriptorSet descriptorWrite = {};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = m_descriptorSets[i];
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pBufferInfo = &bufferInfo;
      descriptorWrite.pImageInfo = nullptr; // Optional
      descriptorWrite.pTexelBufferView = nullptr; // Optional
      vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);

    }



  }

} // namespace fn
