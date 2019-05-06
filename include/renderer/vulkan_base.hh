#if !defined(VULKAN_BASE_H)
/* ========================================================================
   $File: vulkan_base.hh $
   $Date: Fri Apr 26 21:22:20 2019 $
   $Revision: $
   $Creator: Ro Stelmach $
   $Notice: (C) Copyright 2019 by Ro Orestis Stelmach. All Rights Reserved. $
   ======================================================================== */

#define VULKAN_BASE_H

#include <array>
#include <memory>
#include <optional>
#include <vector>

#include "core/logger.hh"
#include "math/vector.hh"
#include "math/matrix.hh"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fn {

// How many frames should be processed concurrently?
  constexpr const int MAX_FRAMES_IN_FLIGHT = 2;

  struct QueueFamilyIndices {

    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  struct Vertex {
    Vec2 position;
    Vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
      VkVertexInputBindingDescription bindingDescription = {};

      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Vertex);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2>
    getAttributesDescriptions() {
      std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertex, position);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertex, color);

      return attributeDescriptions;
    }
  };

  class Settings;

  class VulkanBase {
  public:
    explicit VulkanBase(std::shared_ptr<Settings> settings) noexcept;
    ~VulkanBase() noexcept;

    // TODO: Disable Copy
    // TODO: Make it movable object

    void run() noexcept;

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                  void *pUserData) {

      log::error("Validation layer: %s \n", pCallbackData->pMessage);

      return VK_FALSE;
    }

    static void frameBufferResizedCallback(GLFWwindow *window, int width,
                                           int height) {
      auto app = reinterpret_cast<VulkanBase *>(glfwGetWindowUserPointer(window));
      app->m_frameBufferHasResized = true;
      int a = 10;
    }

  private:
    GLFWwindow *m_window;
    std::shared_ptr<Settings> m_settings;
    VkInstance m_instance;

    VkDebugUtilsMessengerEXT m_debugMessenger;

    // @fix maybe move surface to it's own class?
    VkSurfaceKHR m_surface;

    bool m_enableValidationLayers;

    // Physical Device - GPU -
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    // Logical Device, actually just a handle to
    // physical Device
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkSwapchainKHR m_swapChain;

    // Handles of the VkImages that are reference inside the
    // the swap chain
    std::vector<VkImage> m_swapChainImages;

    // to use any VkImage, including those in the swap chain, in the rendering
    // pipeline we have to create a VkImageView object. An image view is quite
    // literally a view into a image. It describes how to access the image and
    // which part of the image to access
    std::vector<VkImageView> m_swapChainImagesViews;
    std::vector<VkFramebuffer> m_swapChainFrameBuffers;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    VkRenderPass m_renderPass;
    VkDescriptorSetLayout m_descriptorSetLayout;

    VkPipelineLayout m_pipelineLayout;

    VkPipeline m_graphicsPipeline;

    // Command buffers are used to record drawing commands
    std::vector<VkCommandBuffer> m_commandBuffers;

    // Coomand pools manage the memory that is used to store the buffers
    // and command ubffers are allocated from them.
    VkCommandPool m_commandPool;

    // Buffers
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;

    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

   
    // Sempahores are used here for GPU-GPU Synchronization
    struct {
      // Each fraome should have its own semaphore
      std::vector<VkSemaphore> imageIsAvailable;
      std::vector<VkSemaphore> renderHasFinished;
    } m_semaphores;

    // Fences are used for CPU-GPU Synchronization
    std::vector<VkFence> m_inFlightFences;

    // To use the right pair of semaphores every time, we need to keep track
    // of current frame
    size_t m_currentFrame = 0;
    bool m_frameBufferHasResized = false;

    const std::vector<const char *> m_validationLayers = {
      "VK_LAYER_LUNARG_standard_validation"};

    const std::vector<const char *> m_deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    struct UniformBufferObject {
      Matrix4 model;
      Matrix4 view;
      Matrix4 proj;
    };

    /// Rectangle with indicies
    const std::vector<Vertex> vertices = {
      {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
      0, 1, 2, 2, 3, 0
    };

    /// Triagnel
    // const std::vector<Vertex> vertices = {{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    //                                       {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    //                                       {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

    void initWindow() noexcept;
    void initVulkan() noexcept;
    void mainLoop() noexcept;
    void cleanUp() noexcept;

    void createInstance() noexcept;
    void setupDebugMessenger() noexcept;
    void createSurface() noexcept;

    void pickPhysicalDevice() noexcept;
    void createLogicalDevice() noexcept;
    void createSwapChain() noexcept;
    void createImageViews() noexcept;

    void createRenderPass() noexcept;
    void createGraphicsPipeline() noexcept;
    void createFrameBuffers() noexcept;
    void createCommandPool() noexcept;
    void createVertexBuffer() noexcept;
    void createIndexBuffer() noexcept;
    void createCommandBuffers() noexcept;
    void createSyncObjects() noexcept;
    void recreateSwapChain() noexcept;
    void cleanupSwapChain() noexcept;
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory) noexcept;
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) noexcept;

    void createDescriptorSetLayout() noexcept;
    void createUniformBuffers() noexcept;
    void createDescriptorPool() noexcept;
    void createDescriptorSets() noexcept;
    void updateuniformbuffers(uint32_t currentimage) noexcept;

    void drawFrame() noexcept;
    ///@Fix -> maybe move this function out of class.
    /// it is not uses any class memebers anyways
    VkShaderModule createShaderModule(const std::vector<char> &code) const
      noexcept;

    ///@Fix -> maybe move this function outside class?
    bool isDeviceSuitable(VkPhysicalDevice device) const noexcept;

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const noexcept;
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const
      noexcept;

    // Surface format, represents the color depth, e.g color channels and
    // types, also color space
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats) const noexcept;

    // The presenation mode is arguably the most important setting for the swap
    // chain, because it represents the actual conditions for showing images to
    // the screen
    VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes) const
      noexcept;

    // The swap extent is the resolution of images in the swap chain
    // and its *almost* always equal to the window we are drawing in
    VkExtent2D
    chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const noexcept;

    bool checkValidationLayerSupport() const noexcept;
    std::vector<const char *> getRequiredExtensions() const noexcept;
    bool checkDeviceextensionsupport(VkPhysicalDevice device) const noexcept;

    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties) const noexcept;
  };
} // namespace fn

#endif
