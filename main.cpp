#include "application.h"
#include <vulkan/vulkan.h>

int main() {
  VkInstance vkInstance;

  ApplicationConfig config = {
      .name   = "Pokemoon",
      .width  = 240,
      .height = 240,
  };
  application_create(config);
  application_run();
}
