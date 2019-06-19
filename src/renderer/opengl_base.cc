#include "renderer/opengl_base.hh"

#include "core/fission.hh"
#include "core/io_manager.hh"
#include "core/settings.hh"
#include "math/math_utils.hh"
#include "renderer/gl_shader_program.hh"

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <streambuf>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fn {

  void framebuffer_size_callback( GLFWwindow *window, int width, int height ) {
    glViewport( 0, 0, width, height );
  }

  void processInput( GLFWwindow *window ) {

    if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ) {
      glfwSetWindowShouldClose( window, true );
    }
  }

  OpenGLBase::OpenGLBase( std::shared_ptr<Settings> settings ) noexcept
      : m_settings( settings )
      , m_window( nullptr ) {

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
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    m_window = glfwCreateWindow( static_cast<int>( m_settings->getWidth() ),
                                 static_cast<int>( m_settings->getHeight() ),
                                 m_settings->getEngineName().c_str(), nullptr,
                                 nullptr );
    FN_ASSERT_M( m_window, "Faild to create GLFW Window" );

    m_iomanager = IOManager::getInstnace();
    m_iomanager->setWindow( m_window );
  }

  void OpenGLBase::initOpengl() noexcept {

    glfwMakeContextCurrent( m_window );
    // initialize GLAD
    FN_ASSERT_M( gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress ),
                 "Faild to initialize GLAD" );

    // Viewport transforms the 2D coordinates it processed to coordinates
    // on your screen.
    glfwSetFramebufferSizeCallback( m_window, framebuffer_size_callback );
  }

  void OpenGLBase::mainLoop() noexcept {
    // Load shaders

    m_shader->create( "../shaders/gl_texture.vert",
                      "../shaders/gl_texture.frag" );


    unsigned int VBO, VAO, EBO;
    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );
    glGenBuffers( 1, &EBO );

    glBindVertexArray( VAO );

    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices,
                  GL_STATIC_DRAW );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices.data(),
                  GL_STATIC_DRAW );

    // position attribute
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ),
                           ( void * )0 );
    glEnableVertexAttribArray( 0 );
    // texture coord attribute
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ),
                           ( void * )( 3 * sizeof( float ) ) );
    glEnableVertexAttribArray( 1 );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    unsigned int texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    int width, height, nrChannels;
    unsigned char *data = stbi_load( "../textures/container.jpg", &width,
                                     &height, &nrChannels, 0 );

    if ( data ) {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                    GL_UNSIGNED_BYTE, data );
      glGenerateMipmap( GL_TEXTURE_2D );
    } else {
      log::error( "Failed to load texture\n" );
    }
    stbi_image_free( data );

    unsigned int texture2;
    glGenTextures( 1, &texture2 );
    glBindTexture( GL_TEXTURE_2D, texture2 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    stbi_set_flip_vertically_on_load( true );
    data = stbi_load( "../textures/awesomeface.png", &width, &height,
                      &nrChannels, 0 );

    if ( data ) {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, data );
      glGenerateMipmap( GL_TEXTURE_2D );
    } else {
      log::error( "Failed to load texture\n" );
    }
    stbi_image_free( data );

    m_shader->use();
    m_shader->setInt( "texture1", 0 );
    m_shader->setInt( "texture2", 1 );

    glEnable( GL_DEPTH_TEST );

    glm::vec3 cameraPos = glm::vec3( 0.0f, 0.0f, 3.0f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    char str_buffer[ 128 ];

    double lastTime = glfwGetTime();

    while ( !glfwWindowShouldClose( m_window ) ) {

      double currentTime = glfwGetTime();
      double deltaTime = currentTime - lastTime;

      sprintf( str_buffer, "deltaTime: %lf", deltaTime );
      glfwSetWindowTitle( m_window, str_buffer );

      // input
      processInput( m_window );

      // rendering commands here
      renderingCommands();

      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, texture );
      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, texture2 );
      // 4. draw the objects
      m_shader->use();

      float radius = 10.0f;
      float camX = sin( glfwGetTime() ) * radius;
      float camZ = cos( glfwGetTime() ) * radius;

      float cameraSpeed = 2.5f * deltaTime;

      if ( m_iomanager->isKeyPressed( GLFW_KEY_W ) )
        cameraPos += cameraSpeed * cameraFront;
      if ( m_iomanager->isKeyPressed( GLFW_KEY_S ) )
        cameraPos -= cameraSpeed * cameraFront;
      if ( m_iomanager->isKeyPressed( GLFW_KEY_A ) )
        cameraPos -=
            glm::normalize( glm::cross( cameraFront, cameraUp ) ) * cameraSpeed;
      if ( m_iomanager->isKeyPressed( GLFW_KEY_D ) )
        cameraPos +=
            glm::normalize( glm::cross( cameraFront, cameraUp ) ) * cameraSpeed;


      glm::mat4 view;
      view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );

      glm::mat4 projection;
      projection = glm::perspective( glm::radians( 45.0f ),
                                     ( float )m_settings->getWidth() /
                                         m_settings->getHeight(),
                                     0.1f, 100.0f );

      m_shader->setMat4( "view", view );
      m_shader->setMat4( "projection", projection );

      glBindVertexArray( VAO );

      for ( unsigned int i = 0; i < 10; i++ ) {
        glm::mat4 model = glm::mat4( 1.0f );
        model = glm::translate( model, cubePositions[ i ] );
        float angle = 20.0f * i;
        model = glm::rotate( model, glm::radians( angle ),
                             glm::vec3( 1.0f, 0.3f, 0.5f ) );
        m_shader->setMat4( "model", model );
        glDrawArrays( GL_TRIANGLES, 0, 36 );
      }

      glfwSwapBuffers( m_window );
      m_iomanager->update( 0.0f );

      lastTime = currentTime;
    }

    glDeleteVertexArrays( 1, &VAO );
    glDeleteBuffers( 1, &VBO );
    glDeleteBuffers( 1, &EBO );
  }

  void OpenGLBase::renderingCommands() noexcept {

    glad_glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  }

  void OpenGLBase::cleanUp() noexcept {
    IOManager::destory();
    glfwTerminate();
  }

}    // namespace fn
