// performs recursive exponentiation
#include <stdio.h>


int exponentiate_tr(int base, int power, int acc) {
  if ( power == 0 ) return acc;
  if ( power == 1 ) return acc * base;

  return exponentiate_tr(base, power-1, acc*base);
}

int exponentiate(int base, int power) {
  return exponentiate_tr(base, power, 1);
}

int main() {
  printf("2^7: %i\n", exponentiate(2, 7));
  printf("7^5: %i\n", exponentiate(7, 5));
}
