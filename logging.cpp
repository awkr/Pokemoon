//
// Created by Hongjian Zhu on 2022/10/8.
//

#include "logging.h"
#include "defines.h"
#include "platform.h"
#include <chrono>
#include <cstdarg>
#include <cstdio>

u64 get_current_time_in_ms() {
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

void logging_initialize() {}

void logging_shutdown() {}

void log_output(LogLevel level, const char *message, ...) {
  const char *prefixes[6] = {"[Trace]", "[Debug]", "[Info]", "[Warn]", "[Error]", "[Fatal]"};

  const u64 size = 32 * KiB;

  char buffer[size];
  platform_zero_memory(buffer, sizeof(buffer));

  va_list args;
  va_start(args, message);
  vsnprintf(buffer, size, message, args);
  va_end(args);

  char out[size];
  platform_zero_memory(out, sizeof(out));

  sprintf(out, "%s %s\n", prefixes[(u8) level], buffer);

  if (level >= LogLevel::Warn) {
    platform_console_write_error(out);
  } else {
    platform_console_write(out);
  }
}

void report_assertion_failure(CString expression, CString file, u32 line) {
  log_output(
      LogLevel::Fatal, "Assertion failure: %s, in file: %s, line: %d", expression, file, line);
}

void report_assertion_failure(CString expression, CString message, CString file, u32 line) {
  log_output(LogLevel::Fatal,
             "Assertion failure: %s, message: '%s', in file: %s, line: %d",
             expression,
             message,
             file,
             line);
}
