//
// Created by Hongjian Zhu on 2022/10/10.
//

#ifndef POKEMOON_CLOCK_H
#define POKEMOON_CLOCK_H

#include "defines.h"

struct Clock {
  f64 startTime;
  f64 elapsed;
};

// Start the provided clock, reset elapsed time
void clock_start(Clock *clock);

// Update the provided clock, should be called just before checking elapsed time, has no effect on
// non-started clocks.
void clock_update(Clock *clock);

// Stop the provided clock, doesn't reset elapsed time
void clock_stop(Clock *clock);

#endif // POKEMOON_CLOCK_H
