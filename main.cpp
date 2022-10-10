#include "application.h"
#include "memory.h"

int main() {
  memory_initialize();

  ApplicationConfig config = {
      .name   = "Pokemoon",
      .width  = 240,
      .height = 240,
  };
  application_create(config);
  application_run();

  memory_shutdown();
}
