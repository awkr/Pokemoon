//
// Created by Hongjian Zhu on 2022/10/8.
//

#include "logging.h"
#include "defines.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

void logging_initialize() {}

void logging_shutdown() {}

void log_output(LogLevel level, const char *message, ...) {
  const char *prefixes[6] = {"[Trace]", "[Debug]", "[Info]", "[Warn]", "[Error]", "[Fatal]"};

  const u64 size = 32 * KiB;

  char buffer[size];
  memset(buffer, 0, sizeof(buffer));

  va_list args;
  va_start(args, message);
  vsnprintf(buffer, size, message, args);
  va_end(args);

  char out[size];
  memset(out, 0, sizeof(out));

  sprintf(out, "%s %s\n", prefixes[(u8) level], buffer);

  printf("%s", out);
}
