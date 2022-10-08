#include "defines.h"
#include "logging.h"
#include "platform.h"
#include <vulkan/vulkan.h>

int main() {
  VkInstance vkInstance;

  LOG_DEBUG("Hello, Pokemoon.");

  PlatformState platformState{};
  platform_startup(&platformState, "Pokemoon", 240, 240);
  while (true) {
    platform_poll_events(&platformState);
  }
  platform_shutdown(&platformState);
}
