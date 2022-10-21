//
// Created by Hongjian Zhu on 2022/10/10.
//

#include "core/Clock.h"
#include "platform.h"

void Clock::start() {
  _startTime = platform_get_absolute_time();
  _elapsed   = 0;
}

void Clock::tick() {
  if (_startTime != 0) { _elapsed = platform_get_absolute_time() - _startTime; }
}

void Clock::stop() { _startTime = 0; }
