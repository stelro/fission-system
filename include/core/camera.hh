#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fn {

  class Camera {
  private:
    glm::mat4 m_viewMatrix;

    glm::vec3 m_position;
    glm::vec3 m_worldUp;
    glm::vec3 m_up;
    glm::vec3 m_front;

  public:
    Camera( const glm::vec3 &pos = glm::vec3( 0.0f, 0.0f, 3.0f ),
            const glm::vec3 &up = glm::vec3( 0.0f, 1.0f, 0.0f ) ) noexcept;

    void update( float delta ) noexcept;
    void updateCameraVectors() noexcept;

    glm::mat4 view() const noexcept {
      return glm::lookAt( m_position, m_position + m_front, m_up );
    }

    void setFrontVector( const glm::vec3 &front ) noexcept {
      m_front = front;
    }

    void setUpVector( const glm::vec3 &up ) noexcept {
      m_up = up;
    }

    void setPositionVector( const glm::vec3 &pos ) noexcept {
      m_position = pos;
    }

    glm::vec3 front() const noexcept {
      return m_front;
    }

    glm::vec3 up() const noexcept {
      return m_up;
    }

    glm::vec3 position() const noexcept {
      return m_position;
    }
  };

}    // namespace fn
