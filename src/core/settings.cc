/* ========================================================================
   $File: settings.cc $
   $Date: Fri Apr 26 21:38:56 2019 $
   $Revision: $
   $Creator: Ro Stelmach $
   $Notice: (C) Copyright 2019 by Ro Orestis Stelmach. All Rights Reserved. $
   ======================================================================== */
#include "core/settings.hh"

namespace fn {

  Settings::Settings() noexcept
    : m_width(1024)
    , m_height(768)
     { }

  Settings::~Settings() noexcept {}

}
