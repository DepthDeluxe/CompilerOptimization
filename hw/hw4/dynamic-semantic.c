#include <stdio.h>

int main() {
  int i[3];

  // index out of bounds
  printf("int: %i", i[5000000]);
}
