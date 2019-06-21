#ifdef VULKAN_RENDERER
#include "renderer/vulkan_base.hh"
#else
#include "renderer/opengl_base.hh"
#endif
#include "core/engine.hh"
#include "core/fission.hh"
#include "core/settings.hh"

#include "renderer/base_renderer.hh"

#include <algorithm>
#include <vector>

int main() {

  std::shared_ptr<fn::Settings> settings = std::make_shared<fn::Settings>();
  settings->setWidth( 1440 );
  settings->setHeight( 900 );
  settings->setEngineName( "Fission Engine /  Renderer" );

  fn::Engine *engine = fn::Engine::getInstance();
  engine->setRenderer( std::make_shared<fn::VulkanBase>( settings ) );
  engine->run();
  engine->destroy();

  return 0;
}
