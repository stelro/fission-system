#include "core/camera.hh"

namespace fn {

  Camera::Camera( const glm::vec3 &pos, const glm::vec3 &up ) noexcept
      : m_position( pos )
      , m_worldUp( up )
      , m_front( glm::vec3( 0.0f, 0.0f, -1.0f ) ) {}


  void Camera::update( [[maybe_unused]] float delta ) noexcept {}

  void Camera::updateCameraVectors() noexcept {
    glm::vec3 cameraTarget = glm::vec3( 0.0f, 0.0f, 0.0f );
    glm::vec3 cameraDirection = glm::normalize( m_position - cameraTarget );

    glm::vec3 cameraRight =
        glm::normalize( glm::cross( m_worldUp, cameraDirection ) );

    m_up = glm::cross( cameraDirection, cameraRight );
  }


}    // namespace fn
