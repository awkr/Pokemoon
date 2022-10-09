#include "application.h"
#include "memory.h"
#include <vulkan/vulkan.h>

int main() {
  memory_initialize();

  VkInstance vkInstance;

  ApplicationConfig config = {
      .name   = "Pokemoon",
      .width  = 240,
      .height = 240,
  };
  application_create(config);
  application_run();

  memory_shutdown();
}
