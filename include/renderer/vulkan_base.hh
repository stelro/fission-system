#if !defined(VULKAN_BASE_H)
/* ========================================================================
   $File: vulkan_base.hh $
   $Date: Fri Apr 26 21:22:20 2019 $
   $Revision: $
   $Creator: Ro Stelmach $
   $Notice: (C) Copyright 2019 by Ro Orestis Stelmach. All Rights Reserved. $
   ======================================================================== */

#define VULKAN_BASE_H

#include <memory>
#include <vector>
#include <optional>

#include "core/logger.hh"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fn {

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

  class VulkanBase {
  public:
    explicit VulkanBase(std::shared_ptr<Settings> settings) noexcept;
    ~VulkanBase() noexcept;

    //TODO: Disable Copy
    //TODO: Make it movable object

    void run() noexcept;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData) {


      log::error("Validation layer: %s \n", pCallbackData->pMessage);

      return VK_FALSE;
    }


  private:

    GLFWwindow* m_window;
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

    // to use any VkImage, including those in the swap chain, in the rendering pipeline
    // we have to create a VkImageView object. An image view is quite literally a view into
    // a image. It describes how to access the image and which part of the image to access
    std::vector<VkImageView> m_swapChainImagesViews;

    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;

    VkPipeline m_graphicsPipeline;

    const std::vector<const char*> m_validationLayers = {
      "VK_LAYER_LUNARG_standard_validation"
    };

    const std::vector<const char*> m_deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

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

    ///@Fix -> maybe move this function out of class.
    /// it is not uses any class memebers anyways
    VkShaderModule createShaderModule(const std::vector<char>& code) const noexcept;

    ///@Fix -> maybe move this function outside class?
    bool isDeviceSuitable(VkPhysicalDevice device) const noexcept;

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const noexcept;
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const noexcept;

    // Surface format, represents the color depth, e.g color channels and
    // types, also color space
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const noexcept;

    // The presenation mode is arguably the most important setting for the swap chain,
    // because it represents the actual conditions for showing images to the screen
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const noexcept;

    // The swap extent is the resolution of images in the swap chain
    // and its *almost* always equal to the window we are drawing in
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const noexcept;

    bool checkValidationLayerSupport() const noexcept;
    std::vector<const char*> getRequiredExtensions() const noexcept;
    bool checkDeviceextensionsupport(VkPhysicalDevice device) const noexcept;

  };
} //fn

#endif
