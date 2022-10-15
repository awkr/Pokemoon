//
// Created by Hongjian Zhu on 2022/10/9.
//

#ifndef POKEMOON_EVENT_H
#define POKEMOON_EVENT_H

#include "defines.h"

union EventContext { // 128B
  i64  i64[2];
  i32  i32[4];
  i16  i16[8];
  i8   i8[16];
  u64  u64[2];
  u32  u32[4];
  u16  u16[8];
  u8   u8[16];
  f64  f64[2];
  f32  f32[4];
  char c[16];
};

enum class EventCode {
  Unknown = 0x00,
  ApplicationQuit,
  // Context usage:
  // u32[0]: width
  // u32[1]: height
  WindowResized,
  // Context usage:
  // u16 keyCode = .u16[0]
  KeyPressed,
  KeyReleased,
  KeyPressing,
  MouseButtonPressed,
  MouseButtonReleased,
  MouseButtonPressing,
  // Context usage:
  // f32 x = .f32[0];
  // f32 y = .f32[1];
  MouseMoved,
  // Context usage:
  // f32 xOffset = .f32[0];
  // f32 yOffset = .f32[1];
  MouseWheeled,
  Max = 0xFF,
};

// Return true if handled
typedef bool (*PFN_on_event)(EventCode           code,
                             void               *sender,
                             void               *listener,
                             const EventContext &context);

void event_initialize();
void event_shutdown();

bool event_register(EventCode code, void *listener, PFN_on_event onEvent);
bool event_deregister(EventCode code, void *listener, PFN_on_event onEvent);
bool event_fire(EventCode code, void *sender, const EventContext &context);

#endif // POKEMOON_EVENT_H
