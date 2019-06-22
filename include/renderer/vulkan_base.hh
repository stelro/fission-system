#pragma once

#include <array>
#include <memory>
#include <optional>
#include <vector>

#include "core/logger.hh"
#include "math/matrix.hh"
#include "math/vector.hh"
#include "renderer/base_renderer.hh"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof( Vertex );
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3> getAttributesDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

    attributeDescriptions[ 0 ].binding = 0;
    attributeDescriptions[ 0 ].location = 0;
    attributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[ 0 ].offset = offsetof( Vertex, position );

    attributeDescriptions[ 1 ].binding = 0;
    attributeDescriptions[ 1 ].location = 1;
    attributeDescriptions[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[ 1 ].offset = offsetof( Vertex, color );

    attributeDescriptions[ 2 ].binding = 0;
    attributeDescriptions[ 2 ].location = 2;
    attributeDescriptions[ 2 ].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[ 2 ].offset = offsetof( Vertex, texCoord );


    return attributeDescriptions;
  }

  bool operator==( const Vertex &other ) const {
    return position == other.position && color == other.color && texCoord == other.texCoord;
  }
};

namespace std {
  template <>
  struct hash<Vertex> {
    size_t operator()( Vertex const &vertex ) const {
      return ( ( hash<glm::vec3>()( vertex.position ) ^
                 ( hash<glm::vec3>()( vertex.color ) << 1 ) ) >>
               1 ) ^
             ( hash<glm::vec2>()( vertex.texCoord ) << 1 );
    }
  };
}    // namespace std

namespace fn {

  // How many frames should be processed concurrently?

  constexpr const int MAX_FRAMES_IN_FLIGHT = 2;

  const std::string MODEL_PATH = "../models/Crate1.obj";
  const std::string TEXTURE_PATH = "../textures/crate_1.jpg";

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


  class Settings;
  class IOManager;
  class Camera;

  class VulkanBase : public BaseRenderer {
  public:
    explicit VulkanBase( std::shared_ptr<Settings> settings ) noexcept;
    ~VulkanBase() noexcept;

    // TODO: Disable Copy
    // TODO: Make it movable object

    void run() noexcept;

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                   VkDebugUtilsMessageTypeFlagsEXT messageType,
                   const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData ) {

      log::error( "Validation layer: %s \n", pCallbackData->pMessage );

      return VK_FALSE;
    }

    static void frameBufferResizedCallback( GLFWwindow *window, int width, int height ) {
      auto app = reinterpret_cast<VulkanBase *>( glfwGetWindowUserPointer( window ) );
      app->m_frameBufferHasResized = true;
    }

  private:
    GLFWwindow *m_window;
    std::shared_ptr<Settings> m_settings;
    VkInstance m_instance;

    IOManager *m_iomanager = nullptr;
    Camera *m_camera = nullptr;

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

    uint32_t m_mipLevels;
    VkImage m_textureImage;
    VkDeviceMemory m_textureImageMemory;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;


    VkImage m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkImageView m_depthImageView;


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

    const std::vector<const char *> m_validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

    const std::vector<const char *> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    struct UniformBufferObject {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
    };

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    void initWindow() noexcept override;
    void initRenderer() noexcept override;
    //    void mainLoop() noexcept;
    void cleanUp() noexcept override;

    void render( float dt ) noexcept override;
    void update( float dt ) noexcept override;

    GLFWwindow *getWindow() noexcept {
      return m_window;
    }

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
    void loadModel() noexcept;
    void createCommandBuffers() noexcept;
    void createSyncObjects() noexcept;
    void recreateSwapChain() noexcept;
    void cleanupSwapChain() noexcept;
    void createBuffer( VkDeviceSize size, VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags properties, VkBuffer &buffer,
                       VkDeviceMemory &bufferMemory ) noexcept;
    void copyBuffer( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size ) noexcept;

    void createDescriptorSetLayout() noexcept;
    void createUniformBuffers() noexcept;
    void createDescriptorPool() noexcept;
    void createDescriptorSets() noexcept;
    void updateuniformbuffers( uint32_t currentimage ) noexcept;

    void createTextureImage() noexcept;
    void createImage( uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format,
                      VkImageTiling tiling, VkImageUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkImage &image,
                      VkDeviceMemory &imageMemory ) noexcept;
    void createTextureImageView() noexcept;
    VkImageView createImageView( VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                 uint32_t mipLevels ) noexcept;
    void createTextureSampler() noexcept;
    void createDepthResources() noexcept;
    VkFormat findSupportedFormat( const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                  VkFormatFeatureFlags features ) noexcept;
    VkFormat findDepthFormat() noexcept;
    bool hasStencilComponent( VkFormat format ) noexcept;
    void drawFrame() noexcept;

    ///@Fix -> maybe move this function out of class.
    /// it is not uses any class memebers anyways
    VkShaderModule createShaderModule( const std::vector<char> &code ) const noexcept;

    ///@Fix -> maybe move this function outside class?
    bool isDeviceSuitable( VkPhysicalDevice device ) const noexcept;

    QueueFamilyIndices findQueueFamilies( VkPhysicalDevice device ) const noexcept;
    SwapChainSupportDetails querySwapChainSupport( VkPhysicalDevice device ) const noexcept;

    // Surface format, represents the color depth, e.g color channels and
    // types, also color space
    VkSurfaceFormatKHR
    chooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR> &availableFormats ) const
        noexcept;

    // The presenation mode is arguably the most important setting for the swap
    // chain, because it represents the actual conditions for showing images to
    // the screen
    VkPresentModeKHR
    chooseSwapPresentMode( const std::vector<VkPresentModeKHR> &availablePresentModes ) const
        noexcept;

    // The swap extent is the resolution of images in the swap chain
    // and its *almost* always equal to the window we are drawing in
    VkExtent2D chooseSwapExtent( const VkSurfaceCapabilitiesKHR &capabilities ) const noexcept;

    bool checkValidationLayerSupport() const noexcept;
    std::vector<const char *> getRequiredExtensions() const noexcept;
    bool checkDeviceextensionsupport( VkPhysicalDevice device ) const noexcept;

    uint32_t findMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties ) const noexcept;

    VkCommandBuffer beginSingleTimeCommands() noexcept;
    void endSingleTimeCommands( VkCommandBuffer commandBuffer ) noexcept;
    void transitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldlayout,
                                VkImageLayout newlayout, uint32_t mipLevels ) noexcept;
    void copyBuffertoImage( VkBuffer buffer, VkImage image, uint32_t width,
                            uint32_t height ) noexcept;
    void generateMipMaps( VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight,
                                      uint32_t mipLevels ) noexcept;
  };
}    // namespace fn

