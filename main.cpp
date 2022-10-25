#include "application.h"

int main() {
  ApplicationConfig config = {
      .appName = "Pokemoon",
      .width   = 240,
      .height  = 240,
  };
  application_create(config);
  application_run();
}
