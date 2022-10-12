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
  EventCodeEntry registered[(u16) EventCode::Max];
};

static EventSystemState eventSystemState{};

void event_initialize() {}

void event_shutdown() {
  for (auto &entry : eventSystemState.registered) {
    if (entry.events != nullptr) {
      darray_destroy(entry.events);
      entry.events = nullptr;
    }
  }
}

bool event_register(EventCode code, void *listener, PFN_on_event onEvent) {
  auto &entry = eventSystemState.registered[(u16) code];
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
  auto &entry = eventSystemState.registered[(u16) code];
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
  auto &entry = eventSystemState.registered[(u16) code];
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
