#include <iostream>

int main() {
#ifdef DEBUG
  fprintf(stdout, "Hello, Pokemoon.\n");
#endif
}
