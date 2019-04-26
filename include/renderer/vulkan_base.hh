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

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fn {

  class Settings;

  class VulkanBase {
  public:
    explicit VulkanBase(std::shared_ptr<Settings> settings) noexcept;
    ~VulkanBase() noexcept;

    //TODO: Disable Copy
    //TODO: Make it movable object

    void run() noexcept;

  private:

    GLFWwindow* m_window;
    std::shared_ptr<Settings> m_settings;
    VkInstance m_instance;

    void initWindow() noexcept;
    void initVulkan() noexcept;
    void mainLoop() noexcept;
    void cleanUp() noexcept;

    void createInstance() noexcept;

  };

} //fn

#endif
