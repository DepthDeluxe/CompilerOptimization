#include <stdio.h>

// searches through a list recursively to find a value

int testArray[] = {1, 6, 9, 102, 629, 1878, 238759};

int existsInArray(int search, int* array, int len) {
  if ( len == 0 ) return 0;
  if ( len == 1 ) return array[0] == search;

  if ( len % 2 == 0 ) {
    return existsInArray( search, array, len / 2 ) ||
           existsInArray( search, array + (len/2), len / 2);
  } else {
    return existsInArray( search, array, (len-1)/2 ) ||
           existsInArray( search, array + (len-1)/2, (len/2) );
  }
}

int main() {
  printf("search for 6: %i\n", existsInArray(6, testArray, 7));
  printf("search for 10: %i\n", existsInArray(10, testArray, 7));

  return 0;
}
