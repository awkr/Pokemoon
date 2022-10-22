//
// Created by Hongjian Zhu on 2022/10/9.
//

#ifndef POKEMOON_INPUT_H
#define POKEMOON_INPUT_H

#include "defines.h"

enum class Key : u16 {
  Unknown   = 0x00,
  Space     = 32,
  A         = 65,
  C         = 67,
  D         = 68,
  E         = 69,
  M         = 77,
  Q         = 81,
  S         = 83,
  T         = 84,
  Z         = 90,
  ESC       = 256,
  Enter     = 257,
  TAB       = 258,
  Backspace = 259,
  Right     = 262,
  Left      = 263,
  Down      = 264,
  Up        = 265,
};

using MouseButton = u16;

void input_system_initialize(u64 *memorySize, void *pState);
void input_system_shutdown();

void input_update();

// Keyboard input
bool input_is_key_down(Key key);
bool input_is_key_up(Key key);
bool input_was_key_down(Key key);
bool input_was_key_up(Key key);

void input_process_key(Key key, bool pressed);

// Mouse input
bool input_is_mouse_button_down(MouseButton button);
bool input_is_mouse_button_up(MouseButton button);
bool input_was_mouse_button_down(MouseButton button);
bool input_was_mouse_button_up(MouseButton button);

void input_get_mouse_position(f32 &x, f32 &y);
void input_get_previous_mouse_position(f32 &x, f32 &y);

void input_process_mouse_button(MouseButton button, bool pressed);
void input_process_mouse_move(f32 x, f32 y);
void input_process_mouse_wheel(f32 xOffset, f32 yOffset);

#endif // POKEMOON_INPUT_H
