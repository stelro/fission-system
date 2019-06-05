#include "core/fission.hh"
#include "core/settings.hh"
#include "math/math_utils.hh"
#include "math/matrix_transformations.hh"
#include "renderer/gl_shader_program.hh"
#include "renderer/opengl_base.hh"

#include <fstream>
#include <streambuf>
#include <string>

namespace fn {

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

OpenGLBase::OpenGLBase(std::shared_ptr<Settings> settings) noexcept
    : m_settings(settings), m_window(nullptr) {
  m_shader = std::make_unique<ShaderProgram>();
}

OpenGLBase::~OpenGLBase() noexcept {}

void OpenGLBase::run() noexcept {

  initWindow();
  initOpengl();
  mainLoop();
  cleanUp();
}

void OpenGLBase::initWindow() noexcept {

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  m_window =
      glfwCreateWindow(static_cast<int>(m_settings->getWidth()),
                       static_cast<int>(m_settings->getHeight()),
                       m_settings->getEngineName().c_str(), nullptr, nullptr);
  FN_ASSERT_M(m_window, "Faild to create GLFW Window");
}

void OpenGLBase::initOpengl() noexcept {
  glfwMakeContextCurrent(m_window);
  // initialize GLAD
  FN_ASSERT_M(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress),
              "Faild to initialize GLAD");

  // Viewport transforms the 2D coordinates it processed to coordinates
  // on your screen.
  glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
}

void OpenGLBase::mainLoop() noexcept {
  // Load shaders

  m_shader->create("../shaders/gl_texture.vert", "../shaders/gl_texture.frag");

  // Expirimental
  unsigned int VBO;
  // Generate buffer
  // (create the object, and store the reference as ID)
  glGenBuffers(1, &VBO);
  // Bind buffer ( bind the object)

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);

  // 1. Bind Vertex Array Object
  glBindVertexArray(VAO);
  // 2. copy our verticies array in a buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
               GL_STATIC_DRAW);

  // 3. then set our vertex attributes pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  while (!glfwWindowShouldClose(m_window)) {
    // input
    processInput(m_window);

    // rendering commands here
    renderingCommands();

    // 4. draw the objects
    m_shader->use();

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // check and call events and swap buffers
    glfwSwapBuffers(m_window);
    glfwPollEvents();
  }
}

void OpenGLBase::renderingCommands() noexcept {

  glad_glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLBase::cleanUp() noexcept { glfwTerminate(); }

} // namespace fn
