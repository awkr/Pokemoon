//
// Created by Hongjian Zhu on 2022/10/9.
//

#include "event.h"
#include "container/darray.h"
#include "memory.h"

struct RegisteredEvent {
  void        *listener;
  PFN_on_event callback;
};

struct EventCodeEntry {
  RegisteredEvent *events;
};

struct EventSystemState {
  // Lookup table for event codes
  EventCodeEntry registered[EVENT_CODE_COUNT];
};

static EventSystemState *state = nullptr;

void event_system_initialize(u64 *memorySize, void *pState) {
  *memorySize = sizeof(EventSystemState);
  if (!pState) { return; }
  state = (EventSystemState *) pState;
}

void event_system_shutdown() {
  for (auto &entry : state->registered) {
    if (entry.events != nullptr) {
      darray_destroy(entry.events);
      entry.events = nullptr;
    }
  }
  state = nullptr;
}

bool event_register(EventCode code, void *listener, PFN_on_event onEvent) {
  auto &entry = state->registered[(u16) code];
  if (entry.events == nullptr) {
    entry.events = DARRAY_CREATE_TAG(RegisteredEvent, MemoryTag::Event);
  }
  auto registeredCount = darray_length(entry.events);
  for (u32 i = 0; i < registeredCount; ++i) {
    const auto &e = entry.events[i];
    if (e.listener == listener && e.callback == onEvent) { return false; }
  }
  // If at this point, no duplicate was found. Processed with registration.
  RegisteredEvent event = {.listener = listener, .callback = onEvent};
  DARRAY_PUSH(entry.events, event);
  return true;
}

bool event_deregister(EventCode code, void *listener, PFN_on_event onEvent) {
  auto &entry = state->registered[(u16) code];
  if (entry.events == nullptr) { return false; }
  auto registeredCount = darray_length(entry.events);
  for (u32 i = 0; i < registeredCount; ++i) {
    const auto &e = entry.events[i];
    if (e.listener == listener && e.callback == onEvent) {
      darray_pop_at(entry.events, i); // Found, remove it
      return true;
    }
  }
  return false; // Not found
}

bool event_fire(EventCode code, void *sender, const EventContext &context) {
  auto &entry = state->registered[(u16) code];
  if (entry.events == nullptr) { return false; }
  auto registeredCount = darray_length(entry.events);
  for (u32 i = 0; i < registeredCount; ++i) {
    const auto &e = entry.events[i];
    if (e.callback(code, sender, e.listener, context)) {
      return true; // Message has been handled, do not send to other listeners
    }
  }
  return false;
}
