#include "renderer/vulkan_base.hh"
#include "core/settings.hh"
#include "core/fission.hh"
#include <algorithm>

int main() {

  std::shared_ptr<fn::Settings> settings = std::make_shared<fn::Settings>();
  settings->setWidth(1440);
  settings->setHeight(900);
  settings->setEngineName("Fission Engine / Vulkan Renderer");

   fn::VulkanBase vulkanRenderer(settings);
  vulkanRenderer.run();
  FN_ASSERT(false && "Hello world");


  return 0;
}
