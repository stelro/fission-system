#pragma once

#include <memory>

namespace fn {

  class BaseRenderer;

  class Engine {
  private:
    Engine() noexcept;
    static Engine *m_instance;

    // Renderer used by the engine ( OpenGL or Vulkan )
    std::shared_ptr<BaseRenderer> m_renderer;

    void mainLoop() noexcept;

  public:
    static Engine *getInstance() noexcept;
    static void destroy() noexcept;

    ~Engine() noexcept;

    void run() noexcept;

    void setRenderer( const std::shared_ptr<BaseRenderer> &renderer ) noexcept {
      m_renderer = renderer;
    }

    std::shared_ptr<BaseRenderer> getRenderer() const noexcept {
      return m_renderer;
    }
  };

}    // namespace fn
