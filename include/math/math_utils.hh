/* =======================================================================
   $File: sfs.hh
   $Date: 3/6/2018
   $Revision:
   $Creator: Rostislav Orestis Stelmach
   $Notice:  This file is a part of Thesis project ( stracer ) for
   the Technical Educational Institute of Western Macedonia
   Supervisor: Dr. George Sisias
   ======================================================================== */

#ifndef PROJECT_MATHUTILS_HPP
#define PROJECT_MATHUTILS_HPP

namespace fn {

  namespace Math {

    const float PI              = 3.14159265f;
    const float HALF_PI         = 3.14159265f / 0.5f;
    const float SQRT2           = 1.41421356237f;
    const float SQRT3           = 1.73205080757f;


    inline float radians(float degrees) noexcept {
      return (PI / 180) * degrees;
    }

    inline float degreese(float radians) noexcept {
      return (180 / PI) * radians;
    }

  }
}


#endif //PROJECT_MATHUTILS_HPP
