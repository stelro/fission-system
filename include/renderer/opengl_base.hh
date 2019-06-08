#pragma once
#include <glad/include/glad/glad.h>
#include <GLFW/glfw3.h>
#include <array>
#include <glm/glm.hpp>
#include <memory>

namespace fn {

  class Settings;

  class ShaderProgram;
  class IOManager;

  class OpenGLBase {
  public:
    explicit OpenGLBase( std::shared_ptr<Settings> settings ) noexcept;
    ~OpenGLBase() noexcept;

    // TODO: Disable Copy
    // TODO: Make it movable object

    void
    run() noexcept;

  private:
    std::shared_ptr<Settings> m_settings;
    std::unique_ptr<ShaderProgram> m_shader;
    GLFWwindow *m_window;
    IOManager *m_iomanager;

    unsigned int m_shaderProgram;

    float vertices[ 180 ] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
        0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};

    glm::vec3 cubePositions[ 10 ] = {
        glm::vec3( 0.0f, 0.0f, 0.0f ),    glm::vec3( 2.0f, 5.0f, -15.0f ),
        glm::vec3( -1.5f, -2.2f, -2.5f ), glm::vec3( -3.8f, -2.0f, -12.3f ),
        glm::vec3( 2.4f, -0.4f, -3.5f ),  glm::vec3( -1.7f, 3.0f, -7.5f ),
        glm::vec3( 1.3f, -2.0f, -2.5f ),  glm::vec3( 1.5f, 2.0f, -2.5f ),
        glm::vec3( 1.5f, 0.2f, -1.5f ),   glm::vec3( -1.3f, 1.0f, -1.5f )};

    std::array<unsigned int, 6> indices = {0, 1, 3, 1, 2, 3};

    void
    initWindow() noexcept;
    void
    initOpengl() noexcept;
    void
    mainLoop() noexcept;
    void
    cleanUp() noexcept;

    void
    renderingCommands() noexcept;
  };

}    // namespace fn
