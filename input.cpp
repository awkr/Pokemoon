//
// Created by Hongjian Zhu on 2022/10/9.
//

#include "input.h"
#include "event.h"
#include "logging.h"
#include "memory.h"

struct KeyboardState {
  bool keys[512];
};

struct MouseState {
  f32  x;
  f32  y;
  bool buttons[16];
};

struct InputSystemState {
  KeyboardState keyboardCurrent;
  KeyboardState keyboardPrevious;
  MouseState    mouseCurrent;
  MouseState    mousePrevious;
};

static InputSystemState *state = nullptr;

void input_system_initialize(u64 *memorySize, void *pState) {
  *memorySize = sizeof(InputSystemState);
  if (!pState) { return; }
  state = (InputSystemState *) pState;
}

void input_system_shutdown() {}

void input_update() {
  // Copy current states to previous states
  memory_copy(&state->keyboardPrevious, &state->keyboardCurrent, sizeof(KeyboardState));
  memory_copy(&state->mousePrevious, &state->mouseCurrent, sizeof(MouseState));
}

bool input_is_key_down(Key key) { return state->keyboardCurrent.keys[(u16) key]; }

bool input_is_key_up(Key key) { return !state->keyboardCurrent.keys[(u16) key]; }

bool input_was_key_down(Key key) { return state->keyboardPrevious.keys[(u16) key]; }

bool input_was_key_up(Key key) { return !state->keyboardPrevious.keys[(u16) key]; }

void input_process_key(Key key, bool pressed) {
  // Only handled if the state actually changed
  if (state->keyboardCurrent.keys[(u16) key] != pressed) {
    state->keyboardCurrent.keys[(u16) key] = pressed; // Update internal state
    // Fire off an event for immediate processing
    EventContext context = {};
    context.u16[0]       = (u16) key;
    event_fire(pressed ? EventCode::KeyPressed : EventCode::KeyReleased, nullptr, context);
  }
}

bool input_is_mouse_button_down(MouseButton button) { return state->mouseCurrent.buttons[button]; }

bool input_is_mouse_button_up(MouseButton button) { return !state->mouseCurrent.buttons[button]; }

bool input_was_mouse_button_down(MouseButton button) {
  return state->mousePrevious.buttons[button];
}

bool input_was_mouse_button_up(MouseButton button) { return !state->mousePrevious.buttons[button]; }

void input_get_mouse_position(f32 &x, f32 &y) {
  x = state->mouseCurrent.x;
  y = state->mouseCurrent.y;
}

void input_get_previous_mouse_position(f32 &x, f32 &y) {
  x = state->mousePrevious.x;
  y = state->mousePrevious.y;
}

void input_process_mouse_button(MouseButton button, bool pressed) {
  if (state->mouseCurrent.buttons[button] != pressed) {
    state->mouseCurrent.buttons[button] = pressed;

    EventContext context = {};
    context.u16[0]       = button;
    event_fire(
        pressed ? EventCode::MouseButtonPressed : EventCode::MouseButtonReleased, nullptr, context);
  }
}

void input_process_mouse_move(f32 x, f32 y) {
  if (state->mouseCurrent.x != x || state->mouseCurrent.y != y) {
    // LOG_TRACE("Mouse position: %f %f", x, y);

    state->mouseCurrent.x = x;
    state->mouseCurrent.y = y;

    EventContext context = {};
    context.f32[0]       = x;
    context.f32[1]       = y;
    event_fire(EventCode::MouseMoved, nullptr, context);
  }
}

void input_process_mouse_wheel(f32 xOffset, f32 yOffset) {
  EventContext context = {};
  context.f32[0]       = xOffset;
  context.f32[1]       = yOffset;
  event_fire(EventCode::MouseWheeled, nullptr, context);
}
