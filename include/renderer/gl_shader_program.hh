#pragma once

#include <glad/glad.h>
#include <string>

namespace fn {

class ShaderProgram {
private:
  unsigned int m_id;

public:
  ShaderProgram() noexcept = default;
  ShaderProgram(const ShaderProgram &rhs) noexcept = default;
  ShaderProgram &operator=(const ShaderProgram &rhs) noexcept = default;
  ShaderProgram(ShaderProgram &&rhs) noexcept = default;
  ShaderProgram &operator=(ShaderProgram &&rhs) noexcept = default;

  void create(const std::string &vertexPath,
              const std::string &fragmentPath) noexcept;

  void destory() noexcept;

  void use() noexcept;
  unsigned int id() const noexcept { return m_id; }

  void setBool(const std::string &name, bool value) const noexcept;
  void setInt(const std::string &name, int value) const noexcept;
  void setFloat(const std::string &name, float value) const noexcept;
};

} // namespace fn
