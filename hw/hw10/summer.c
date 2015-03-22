#include <stdio.h>

int countUp(int start) {
  if ( start == 0 ) return 0;

  return start + countUp(start-1);
}


int main() {
  // test the program
  printf("countUp(10): %i\n", countUp(10));
  printf("countUp(1): %i\n", countUp(1));
}
