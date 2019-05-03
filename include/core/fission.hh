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

#define FN_ASSERT_M(condition, msg) do                               \
  { if (!(condition)) { fn::log::fatal("%s(%d): Assertion Failed with %s\n", __FILE__, __LINE__, msg); } \
  } while(0);

#define FN_ASSERT(condition) do                                        \
  { if (!(condition)) { fn::log::fatal("%s(%d): Assertion Failed\n", __FILE__, __LINE__); } \
  } while(0);


namespace fn {
  std::string VulkanErrorString(VkResult errorCode) noexcept ;
}

#endif
