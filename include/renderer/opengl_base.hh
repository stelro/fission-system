#pragma once
#include <glad/include/glad/glad.h>

#include <GLFW/glfw3.h>
#include <array>
#include <memory>


namespace fn {

class Settings;
class ShaderProgram;

class OpenGLBase {
public:
  explicit OpenGLBase(std::shared_ptr<Settings> settings) noexcept;
  ~OpenGLBase() noexcept;

  // TODO: Disable Copy
  // TODO: Make it movable object

  void run() noexcept;

private:
  std::shared_ptr<Settings> m_settings;
  std::unique_ptr<ShaderProgram> m_shader;
  GLFWwindow *m_window;



  unsigned int m_shaderProgram;

  std::array<float, 18> vertices = {
      // positions        // colors
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom right
      0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left
      0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, // top
  };

  void initWindow() noexcept;
  void initOpengl() noexcept;
  void mainLoop() noexcept;
  void cleanUp() noexcept;

  void renderingCommands() noexcept;
};

} // namespace fn
