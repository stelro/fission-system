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

#include <vector>

namespace fn {

  VulkanBase::VulkanBase(std::shared_ptr<Settings> settings) noexcept
    : m_window(nullptr)
    , m_settings(settings)
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

    m_window = glfwCreateWindow(m_settings->getWidth(),
                                m_settings->getHeight(),
                                m_settings->getEngineName().c_str(),
                                nullptr,
                                nullptr);
  }

  void VulkanBase::initVulkan() noexcept {
    createInstance();
  }

  void VulkanBase::mainLoop() noexcept {

    while (!glfwWindowShouldClose(m_window)) {
      glfwPollEvents();
    }

  }

  void VulkanBase::cleanUp() noexcept {
    vkDestroyInstance(m_instance, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }

  void VulkanBase::createInstance() noexcept {

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

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    createInfo.enabledLayerCount = 0;

    VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &m_instance));

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

  }

} //fn
