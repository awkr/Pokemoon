//
// Created by Hongjian Zhu on 2022/10/8.
//

#ifndef POKEMOON_LOGGING_H
#define POKEMOON_LOGGING_H

#include "defines.h"

enum class LogLevel { Trace = 0, Debug, Info, Warn, Error, Fatal };

void logging_system_initialize(u64 *memorySize, void *pState);
void logging_system_shutdown();

void log_output(LogLevel level, const char *message, ...);

#ifdef DEBUG
#define LOG_TRACE(message, ...) log_output(LogLevel::Trace, message, ##__VA_ARGS__);
#define LOG_DEBUG(message, ...) log_output(LogLevel::Debug, message, ##__VA_ARGS__);
#else
#define LOG_TRACE(message, ...)
#define LOG_DEBUG(message, ...)
#endif
#define LOG_INFO(message, ...)  log_output(LogLevel::Info, message, ##__VA_ARGS__);
#define LOG_WARN(message, ...)  log_output(LogLevel::Warn, message, ##__VA_ARGS__);
#define LOG_ERROR(message, ...) log_output(LogLevel::Error, message, ##__VA_ARGS__);
#define LOG_FATAL(message, ...) log_output(LogLevel::Fatal, message, ##__VA_ARGS__);

#endif // POKEMOON_LOGGING_H
