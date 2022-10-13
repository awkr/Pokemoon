//
// Created by Hongjian Zhu on 2022/10/7.
//

#ifndef POKEMOON_DEFINES_H
#define POKEMOON_DEFINES_H

#include <cstdint>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;

const u64 KiB = 1024;
const u64 MiB = KiB * 1024;
const u64 GiB = MiB * 1024;

using CString = const char *;

void report_assertion_failure(CString expression, CString file, u32 line);

void report_assertion_failure(CString expression, CString message, CString file, u32 line);

#define ASSERT(expr)                                                                               \
  if (expr) {                                                                                      \
  } else {                                                                                         \
    report_assertion_failure(#expr, __FILE_NAME__, __LINE__);                                      \
    __builtin_trap();                                                                              \
  }

#define ASSERT_MESSAGE(expr, message)                                                              \
  if (expr) {                                                                                      \
  } else {                                                                                         \
    report_assertion_failure(#expr, message, __FILE_NAME__, __LINE__);                             \
    __builtin_trap();                                                                              \
  }

#if defined(__APPLE__)
#define PLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define PLATFORM_IOS 1
#elif TARGET_IPHONE_SIMULATOR
#define PLATFORM_IOS_SIMULATOR 1
#endif
#else
#error "Unknown platform"
#endif

#define CLAMP(value, min, max) (value <= min) ? value : ((value <= max) ? value : max)

#endif // POKEMOON_DEFINES_H
