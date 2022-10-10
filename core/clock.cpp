//
// Created by Hongjian Zhu on 2022/10/10.
//

#include "clock.h"
#include "platform.h"

void clock_start(Clock *clock) {
  clock->startTime = platform_get_absolute_time();
  clock->elapsed   = 0;
}

void clock_update(Clock *clock) {
  if (clock->startTime != 0) { clock->elapsed = platform_get_absolute_time() - clock->startTime; }
}

void clock_stop(Clock *clock) { clock->startTime = 0; }
