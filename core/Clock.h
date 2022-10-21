//
// Created by Hongjian Zhu on 2022/10/10.
//

#pragma once

#include "defines.h"

class Clock {
public:
  // Start the clock, reset elapsed time
  void start();

  // Update the clock, should be called before checking elapsed time, has no effect if not started
  void tick();

  // Stop the clock, doesn't reset elapsed time
  void stop();

  // Returns elapsed time in seconds, has no effect if not started
  f64 elapsed() const { return _elapsed; }

private:
  f64 _startTime;
  f64 _elapsed;
};
