#include "core/engine.hh"
#include "core/fission.hh"
#include "core/logger.hh"
#include "renderer/base_renderer.hh"

namespace fn {

  Engine *Engine::m_instance = nullptr;
  Engine::Engine() noexcept {}

  Engine::~Engine() noexcept {}

  Engine *Engine::getInstance() noexcept {

    if ( !m_instance ) {
      m_instance = new Engine();
    }

    return m_instance;
  }

  void Engine::destroy() noexcept {
    if ( m_instance ) {
      delete m_instance;
      m_instance = nullptr;
    } else {
      log::fatal( "Attempted to destroy null engine object" );
    }
  }

  void Engine::mainLoop() noexcept {
    while ( !m_renderer->getShouldTerminate() ) {
      float dt = 1.0f;

      m_renderer->render( dt );
      m_renderer->update( dt );
    }
  }

  void Engine::run() noexcept {
    m_renderer->initWindow();
    m_renderer->initRenderer();
    this->mainLoop();
    m_renderer->cleanUp();
  }
}    // namespace fn
