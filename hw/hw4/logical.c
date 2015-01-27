#include <stdio.h>

// function that adds numbers... Oops, got confused between
// addition and subtraction
int add_numbers(int a, int b) {
  return a - b;
}

int main() {
  printf("1 + 1: %i", add_numbers(1,1));
}
