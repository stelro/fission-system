#ifdef VULKAN_RENDERER
#include "renderer/vulkan_base.hh"
#else
#include "renderer/opengl_base.hh"
#endif
#include "core/fission.hh"
#include "core/settings.hh"
#include <algorithm>
#include <vector>

int main() {

  std::shared_ptr<fn::Settings> settings = std::make_shared<fn::Settings>();
  settings->setWidth( 1440 );
  settings->setHeight( 900 );
  settings->setEngineName( "Fission Engine /  Renderer" );

  fn::VulkanBase vulkanRenderer( settings );
  vulkanRenderer.run();



  // fn::OpenGLBase openglRenderer(settings);
  // openglRenderer.run();
  //
  return 0;
}
