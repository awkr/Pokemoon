//
// Created by Hongjian Zhu on 2022/10/22.
//

#pragma once

#include "defines.h"
#include <cstdio>

namespace utils {

u32 string_length(CString s);

u32 string_format(char *dst, const char *format, ...);
u32 string_format_v(char *dst, const char *format, va_list args);

} // namespace utils
