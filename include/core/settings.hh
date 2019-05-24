#if !defined(SETTINGS_H)
/* ========================================================================
   $File: settings.hh $
   $Date: Fri Apr 26 21:39:27 2019 $
   $Revision: $
   $Creator: Ro Stelmach $
   $Notice: (C) Copyright 2019 by Ro Orestis Stelmach. All Rights Reserved. $
   ======================================================================== */

#define SETTINGS_H

#include <cstdint>
#include <string>

namespace fn {

  class Settings {
  public:
    Settings() noexcept;
    ~Settings() noexcept;

    ///
    /// Setters
    ///
    constexpr void setWidth(uint32_t width) noexcept {
      m_width = width;
    }

    constexpr void setHeight(uint32_t height) noexcept {
      m_height = height;
    }

   void setEngineName(const std::string& name) noexcept {
      m_engineName = name;
    }

    ///
    /// Getters
    ///

    constexpr uint32_t getWidth() const noexcept {
      return m_width;
    }

    constexpr uint32_t getHeight() const noexcept {
      return m_height;
    }

    std::string getEngineName() const noexcept {
      return m_engineName;
    }

  private:
    uint32_t m_width;
    uint32_t m_height;

    std::string m_engineName;
  };

}

#endif
