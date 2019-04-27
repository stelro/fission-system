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

    bool isComplete() {
      return graphicsFamily.has_value();
    }
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

    bool m_enableValidationLayers;

    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;


    const std::vector<const char*> m_validationLayers = {
      "VK_LAYER_LUNARG_standard_validation"
    };

    void initWindow() noexcept;
    void initVulkan() noexcept;
    void mainLoop() noexcept;
    void cleanUp() noexcept;

    void createInstance() noexcept;
    void setupDebugMessenger() noexcept;

    void pickPhysicalDevice() noexcept;

    //@Fix maybe move this function outside class?
    bool isDeviceSuitable(VkPhysicalDevice device) const noexcept;

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const noexcept;

    bool checkValidationLayerSupport() const noexcept;
    std::vector<const char*> getRequiredExtensions() const noexcept;

  };
} //fn

#endif
