#include "core/fission.hh"
#include "core/logger.hh"
#include "renderer/gl_shader_program.hh"

#include <fstream>
#include <iostream>
#include <sstream>

namespace fn {

  void ShaderProgram::create(const std::string &vertexPath,
                             const std::string &fragmentPath) noexcept {

    std::ifstream streamVertexShader;
    streamVertexShader.open(vertexPath);
    FN_ASSERT_M(streamVertexShader.is_open(),
                "Could not open gl_texture.vert file");

    std::string vertexShaderString;
    streamVertexShader.seekg(0, std::ios::end);
    vertexShaderString.reserve(streamVertexShader.tellg());
    streamVertexShader.seekg(0, std::ios::beg);

    vertexShaderString.assign(
      (std::istreambuf_iterator<char>(streamVertexShader)),
      std::istreambuf_iterator<char>());

    streamVertexShader.close();

    std::ifstream streamFragmentshader;
    streamFragmentshader.open(fragmentPath);
    FN_ASSERT_M(streamFragmentshader.is_open(),
                "Could not open gl_texture.frag file");

    std::string fragmentShaderString;
    streamFragmentshader.seekg(0, std::ios::end);
    fragmentShaderString.reserve(streamFragmentshader.tellg());
    streamFragmentshader.seekg(0, std::ios::beg);

    fragmentShaderString.assign(
      (std::istreambuf_iterator<char>(streamFragmentshader)),
      std::istreambuf_iterator<char>());

    streamFragmentshader.close();

    // Compile the actuall shaders

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    const char *vertexShaderSource = vertexShaderString.c_str();
    // Attach the source code to the shader object and comiple the shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if ( !success ) {
      glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
      fn::log::error("Shader vertex compilation failed %s\n", infoLog);
    }

    // Compile the fragment shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentShaderSource = fragmentShaderString.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if ( !success ) {
      glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
      log::error("Shader fragment compilation failed %s\n", infoLog);
    }

    m_id = glCreateProgram();
    glAttachShader(m_id, vertexShader);
    glAttachShader(m_id, fragmentShader);
    glLinkProgram(m_id);

    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if ( !success ) {
      glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
      log::error("Shader program linking faild %s\n", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }

  void ShaderProgram::destory() noexcept {
    // Empty
  }

  void ShaderProgram::use() noexcept { glUseProgram(m_id); }

  void ShaderProgram::setBool(const std::string &name, bool value) const noexcept {

    glUniform1i(glGetUniformLocation(m_id, name.c_str()),
                static_cast<int>(value));
  }

  void ShaderProgram::setInt(const std::string &name, int value) const noexcept {

    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
  }

  void ShaderProgram::setFloat(const std::string &name, float value) const noexcept {

    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
  }

  void ShaderProgram::setMat4(const std::string &name, const glm::mat4 &mat) const noexcept {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }

} // namespace fn
