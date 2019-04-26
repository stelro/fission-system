/* ========================================================================
   $File: vulkan_base.cc $
   $Date: Fri Apr 26 21:22:44 2019 $
   $Revision: $
   $Creator: Ro Stelmach $
   $Notice: (C) Copyright 2019 by Ro Orestis Stelmach. All Rights Reserved. $
   ======================================================================== */

#include "renderer/vulkan_base.hh"

namespace fn {

  VulkanBase::VulkanBase() noexcept
    : m_window(nullptr)
  {
    //Empty Constructor
  }

  VulkanBase::~VulkanBase() noexcept {
    //Empty Destructor
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
  }

  void VulkanBase::initVulkan() noexcept {

  }

  void VulkanBase::mainLoop() noexcept {

    while (!glfwWindowShouldClose(m_window)) {
      glfwPollEvents();
    }

  }

  void VulkanBase::cleanUp() noexcept {
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }

} //fn
