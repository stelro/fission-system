#pragma once

// Project Headers
#include "core/fission.hh"

// GLFW Headers
#include <GLFW/glfw3.h>

// C++ Headers
#include <unordered_map>

namespace fn {

  class IOManager {
  private:
    IOManager() = default;
    static IOManager *m_instance;
    GLFWwindow *m_window;

    std::unordered_map<int, bool> m_pressedKeys;
    std::unordered_map<int, bool> m_previousKeys;

    struct
    {
      double x;
      double y;
    } m_mousePos;

    struct
    {
      bool left = false;
      bool right = false;
      bool middle = false;
    } m_mouseButtons;

    bool
    wasKeyDown( int key ) const noexcept;

    static void
    key_callback( GLFWwindow *window, int key, int scan_code, int action,
                  int modes ) noexcept;
    static void
    mouse_callback( GLFWwindow *window, int button, int action, int mods ) noexcept;

  public:
    ~IOManager() = default;

    static IOManager *
    getInstnace() noexcept;
    static void
    destory() noexcept;

    FN_DISABLE_COPY( IOManager )
    FN_DISABLE_MOVE( IOManager )

    void
    setWindow( GLFWwindow *window ) noexcept;
    void
    update( float dt ) noexcept;

    void
    pressKey( int key ) noexcept;
    void
    releaseKey( int key ) noexcept;

    void
    pressLeftMouse( bool value ) noexcept;
    void
    pressRightMouse( bool value ) noexcept;
    void
    pressMiddleMouse( bool value ) noexcept;

    bool
    isKeyPressed( int key ) const noexcept;
    bool
    isKeyHoldDown( int key ) const noexcept;

    bool
    isLeftMousePressed() const noexcept;
    bool
    isRightMousePressed() const noexcept;
    bool
    isMiddleMousePressed() const noexcept;

    double
    getMousePosX() const noexcept;
    double
    getMousePosY() const noexcept;
  };

}    // namespace fn
