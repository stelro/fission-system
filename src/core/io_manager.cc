#include "core/io_manager.hh"

namespace fn {

  IOManager *IOManager::m_instance = nullptr;

  IOManager *
  IOManager::getInstnace() noexcept {
    if ( m_instance )
      return m_instance;

    m_instance = new IOManager();
    return m_instance;
  }

  void
  IOManager::destory() noexcept {
    FN_ASSERT_M( m_instance, "IOManager is already destroyed!" );
    delete m_instance;
    m_instance = nullptr;
  }

  void
  IOManager::setWindow( GLFWwindow *window ) noexcept {
    m_window = window;
  }

  void
  IOManager::update( [[maybe_unused]] float dt ) noexcept {
    //@fix: this assertion check here is bad, try to solve it
    FN_ASSERT_M( m_window,
                 "Window ojbect has not been set in IOManager class" );

    glfwPollEvents();

    for ( auto &i : m_pressedKeys ) {
      m_previousKeys[ i.first ] = i.second;
    }

    // Handle special case, wehere we should close window with ESC key
    // @fix: check if this key isn't registered by user
    if ( glfwGetKey( m_window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ) {
      glfwSetWindowShouldClose( m_window, true );
    }


    glfwSetWindowUserPointer( m_window, this );
    glfwSetKeyCallback( m_window, key_callback );
    glfwSetMouseButtonCallback( m_window, mouse_callback );

    glfwGetCursorPos( m_window, &m_mousePos.x, &m_mousePos.y );
  }


  void
  IOManager::key_callback( GLFWwindow *window, int key,
                           [[maybe_unused]] int scan_code, int action,
                           [[maybe_unused]] int modes ) noexcept {

    auto *io_manager =
        static_cast<IOManager *>( glfwGetWindowUserPointer( window ) );

    switch ( action ) {
      case GLFW_PRESS:
        io_manager->pressKey( key );
        break;
      case GLFW_RELEASE:
        io_manager->releaseKey( key );
        break;
      case GLFW_REPEAT:
        break;
      default:
        break;
    }
  }

  void
  IOManager::mouse_callback( GLFWwindow *window, int button, int action,
                             int mods ) noexcept {

    auto *io_manager =
        static_cast<IOManager *>( glfwGetWindowUserPointer( window ) );
    auto flag = ( action != GLFW_RELEASE );

    switch ( button ) {
      case GLFW_MOUSE_BUTTON_RIGHT:
        io_manager->pressRightMouse( flag );
        break;
      case GLFW_MOUSE_BUTTON_LEFT:
        io_manager->pressLeftMouse( flag );
        break;
      case GLFW_MOUSE_BUTTON_MIDDLE:
        io_manager->pressMiddleMouse( flag );
        break;
    }
  }

  bool
  IOManager::wasKeyDown( int key ) const noexcept {
    auto it = m_previousKeys.find( key );

    if ( it != m_previousKeys.end() ) {
      return ( it->second );
    } else {
      return ( false );
    }
  }

  void
  IOManager::pressKey( int key ) noexcept {
    m_pressedKeys[ key ] = true;
  }

  void
  IOManager::releaseKey( int key ) noexcept {
    m_pressedKeys[ key ] = false;
  }

  void
  IOManager::pressLeftMouse( bool value ) noexcept {
    m_mouseButtons.left = value;
  }

  void
  IOManager::pressRightMouse( bool value ) noexcept {
    m_mouseButtons.right = value;
  }

  void
  IOManager::pressMiddleMouse( bool value ) noexcept {
    m_mouseButtons.middle = value;
  }

  bool
  IOManager::isKeyPressed( int key ) const noexcept {
    return isKeyHoldDown( key );
  }

  bool
  IOManager::isKeyHoldDown( int key ) const noexcept {
    auto it = m_pressedKeys.find( key );

    if ( it != m_pressedKeys.end() ) {
      return ( it->second );
    } else {
      return ( false );
    }
  }

  bool
  IOManager::isLeftMousePressed() const noexcept {
    return m_mouseButtons.left;
  }

  bool
  IOManager::isRightMousePressed() const noexcept {
    return m_mouseButtons.right;
  }

  bool
  IOManager::isMiddleMousePressed() const noexcept {
    return m_mouseButtons.middle;
  }

  double
  IOManager::getMousePosX() const noexcept {
    return m_mousePos.x;
  }

  double
  IOManager::getMousePosY() const noexcept {
    return m_mousePos.y;
  }

}    // namespace fn
