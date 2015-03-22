#include <stdio.h>


int countUp_tr(int start, int acc) {
  if ( start == 0 ) return acc;

  return countUp_tr(start-1, acc + start);
}

int countUp(int start) {
  countUp_tr(start, 0);
}


int main() {
  // test the program
  printf("countUp(10): %i\n", countUp(10));
  printf("countUp(1): %i\n", countUp(1));
}
