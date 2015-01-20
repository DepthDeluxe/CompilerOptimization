#include <stdio.h>

int memoizedUarray[30];

void initUarray(int index) {
  if (index >= 30) { return; }

  if (index == 0) {
    memoizedUarray[index] = 0;
  }
  if (index == 1) {
    memoizedUarray[index] = 1;
  }
  if (index > 1) {
    memoizedUarray[index] = -1;
  }

  initUarray(index + 1);
}

int recursiveUfib(int num) {
  if (memoizedUarray[num] > -1) {
    return memoizedUarray[num];
  }

  memoizedUarray[num] = recursiveUfib(num - 1) + recursiveUfib(num - 2);

  return memoizedUarray[num];
}

int outputUfibs(int num) {
  if (num >= 30) { return -1; }

  printf("val: %i\n", recursiveUfib(num) );
  outputUfibs(num + 1);

  return 0;
}

void main(void) {
  initUarray(0);

  outputUfibs(0);
}
