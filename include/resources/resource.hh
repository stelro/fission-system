/* =======================================================================
   $File: myfile.c
   $Date: 21-06-2019
   $Revision: 21-06-2019
   $Creator: Orestis Ro Stelmach
   $Email: stelmach.ro[at]gmail.com
   $Notice:
   ======================================================================== */

#pragma once

#include "core/object.hh"

namespace fn {

  class Resource : public Object {
  private:
  public:
    Resource() noexcept;
    virtual ~Resource() noexcept;
  };

}    // namespace fn
