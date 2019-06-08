#if !defined(FISSION_H)
/* ========================================================================
   $File: fission.hh $
   $Date: Fri Apr 26 21:20:57 2019 $
   $Revision: $
   $Creator: Ro Stelmach $
   $Notice: (C) Copyright 2019 by Ro Orestis Stelmach. All Rights Reserved. $
   ======================================================================== */

#define FISSION_H

#include "core/logger.hh"
#include "vulkan/vulkan.h"

namespace fn {

/*
 * Types aliases, for code portability
 */

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;
using uptr = uintptr_t;
using usize = size_t;

#define VK_CHECK_RESULT(f)                                                     \
  {                                                                            \
    VkResult res = (f);                                                        \
    if (res != VK_SUCCESS) {                                                   \
      fn::log::fatal("%s(%d) : VkResult is %s\n", __FILE__, __LINE__,          \
                     fn::VulkanErrorString(res).c_str());                      \
    }                                                                          \
  }

#define FN_ASSERT_M(condition, msg)                                            \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fn::log::fatal("%s(%d): Assertion failed with error - %s\n", __FILE__,   \
                     __LINE__, msg);                                           \
    }                                                                          \
  } while (0);

#define FN_ASSERT(condition)                                                   \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fn::log::fatal("%s(%d): Assertion failed\n", __FILE__, __LINE__);        \
    }                                                                          \
  } while (0);

#define FN_DISABLE_COPY(Class)                                                 \
  Class(const Class &) = delete;                                               \
  Class &operator=(const Class &) = delete;

#define FN_DISABLE_MOVE(Class)                                                 \
  Class(const Class &&) = delete;                                              \
  Class &operator=(const Class &&) = delete;

std::string VulkanErrorString(VkResult errorCode) noexcept;

} // namespace fn

#endif
