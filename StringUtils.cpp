//
// Created by Hongjian Zhu on 2022/10/22.
//

#include "StringUtils.h"
#include "memory.h"
#include <cstdarg>
#include <cstring>

namespace utils {

u32 string_length(CString s) { return strlen(s); }

u32 string_format(char *dst, const char *format, ...) {
  va_list args;
  va_start(args, format);

  auto written = string_format_v(dst, format, args);

  va_end(args);
  return written;
}

u32 string_format_v(char *dst, const char *format, va_list args) {
  char buffer[32 * KiB]; // Big, but can fit on the stack.
  i32  written    = vsnprintf(buffer, 32 * KiB, format, args);
  buffer[written] = 0;
  memory_copy(dst, buffer, written + 1);
  return written;
}

} // namespace utils
