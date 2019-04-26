#include "renderer/vulkan_base.hh"
#include "core/settings.hh"
#include <algorithm>

int main() {

  std::shared_ptr<fn::Settings> settings = std::make_shared<fn::Settings>();
  settings->setWidth(1024);
  settings->setHeight(768);

  fn::VulkanBase vulkanRenderer(settings);
  vulkanRenderer.run();

  return 0;
}
