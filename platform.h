//
// Created by Hongjian Zhu on 2022/10/8.
//

#ifndef POKEMOON_PLATFORM_H
#define POKEMOON_PLATFORM_H

#include "defines.h"

struct PlatformState {
  void *internalState;
};

void platform_startup(PlatformState *state, CString name, u32 width, u32 height);

void platform_shutdown(PlatformState *state);

void platform_poll_events(PlatformState *state);

void *platform_allocate(u64 size, bool aligned = false);

void platform_free(void *block, bool aligned = false);

void *platform_zero_memory(void *block, u64 size);

void *platform_copy_memory(void *dst, const void *src, u64 size);

void *platform_set_memory(void *dst, i32 value, u64 size);

void platform_console_write(CString message);

void platform_console_write_error(CString message);

f64 platform_get_absolute_time();

void platform_sleep(u64 ms);

void platformGetFramebufferSize(PlatformState *state, u32 &width, u32 &height);

#endif // POKEMOON_PLATFORM_H
