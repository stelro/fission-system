#if !defined(FISSION_H)
/* ========================================================================
   $File: fission.hh $
   $Date: Fri Apr 26 21:20:57 2019 $
   $Revision: $
   $Creator: Ro Stelmach $
   $Notice: (C) Copyright 2019 by Ro Orestis Stelmach. All Rights Reserved. $
   ======================================================================== */

#define FISSION_H

#include "vulkan/vulkan.h"
#include "core/logger.hh"

#define VK_CHECK_RESULT(f) \
{\
  VkResult res = (f);\
  if ( res != VK_SUCCESS ) \
  { \
    fn::log::fatal("VkResult is \" %s \" in %s at line %d", \
                   fn::VulkanErrorString(res).c_str(), __FILE__, __LINE__); \
  }\
}\

namespace fn {
  std::string VulkanErrorString(VkResult errorCode) noexcept ;
}

#endif
