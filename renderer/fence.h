//
// Created by Hongjian Zhu on 2022/10/15.
//

#ifndef POKEMOON_FENCE_H
#define POKEMOON_FENCE_H

#include "renderer/types.h"

void fence_create(Context *context, bool isSignaled, Fence *outFence);

void fence_destroy(Context *context, Fence *fence);

bool fence_wait(Context *context, Fence *fence, u64 timeoutNs = UINT64_MAX);

void fence_reset(Context *context, Fence *fence);

#endif // POKEMOON_FENCE_H
