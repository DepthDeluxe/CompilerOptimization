// performs recursive exponentiation
#include <stdio.h>

int exponentiate(int base, int power) {
  if ( power == 0 ) return 1;
  if ( power == 1 ) return base;

  return base * exponentiate(base, power-1);
}

int main() {
  printf("2^7: %i\n", exponentiate(2, 7));
  printf("7^5: %i\n", exponentiate(7, 5));
}
