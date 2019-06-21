#pragma once


namespace fn {

  class BaseRenderer {
  public:
    BaseRenderer() noexcept;
    virtual ~BaseRenderer() noexcept;
    virtual void initRenderer() noexcept = 0;
    virtual void initWindow() noexcept = 0;
    virtual void cleanUp() noexcept = 0;
    virtual void render( float dt ) noexcept = 0;
    virtual void update( float dt ) noexcept = 0;

    bool getShouldTerminate() const noexcept {
      return p_shouldTerminate;
    }

  protected:
    bool p_shouldTerminate;
  };
}    // namespace fn
