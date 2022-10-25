//
// Created by Hongjian Zhu on 2022/10/8.
//

#include "logging.h"
#include "StringUtils.h"
#include "platform.h"
#include "platform/filesystem.h"
#include <chrono>
#include <cstdarg>
#include <cstdio>

struct LoggerSystemState {
  FileHandle logFileHandle;
};

static LoggerSystemState *state = nullptr;

u64 get_current_time_in_ms() {
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

void append_to_log_file(CString message) {
  if (state) { // Since the message already contains a '\n', just write the bytes directly
    u64 length  = utils::string_length(message);
    u64 written = 0;
    if (!filesystem_write(&state->logFileHandle, length, message, &written)) {
      platform_console_write_error("[Logging] Failed to write log to file");
    }
  }
}

void logging_system_initialize(u64 *memorySize, void *pState) {
  *memorySize = sizeof(LoggerSystemState);
  if (!pState) { return; }
  state = (LoggerSystemState *) pState;
  // Create new/wipe existing log file, then open it
  ASSERT(filesystem_open("pokemoon.log", FILE_MODE_WRITE, false, &state->logFileHandle));
}

void logging_system_shutdown() { state = nullptr; }

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

  utils::string_format(out, "%s %s\n", prefixes[(u8) level], buffer);

  if (level >= LogLevel::Warn) {
    platform_console_write_error(out);
  } else {
    platform_console_write(out);
  }

  append_to_log_file(out);
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
