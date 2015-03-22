// recursive implementation of merge sort
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int testArray[] = {23, 25, 6, 12, 819, 16, 6, 1};

void quickSort(int* array, int len) {
  if ( len == 1 || len == 0 ) return;
  else {
    // pick a pivot
    int pivot = array[0];

    // define a new array
    int* newArray = calloc(len, sizeof(int));
    int numLess = 0;
    int numGreater = 0;

    // partition the values into the new array
    for( int n = 1; n < len; n++ ) {
      int val = array[n];

      if ( val <= pivot ) {
        newArray[numLess++] = val;
      } else {
        newArray[len - ++numGreater] = val;
      }
    }

    // put the pivot in the middle
    newArray[numLess] = pivot;

    // copy values back into original array and free the temp array
    memcpy(array, newArray, len * sizeof(int));
    free(newArray);

    // run quicksort on each of the partitions
    quickSort(array, numLess);
    quickSort(array + numLess + 1, numGreater);
  }
}

int main() {
  quickSort(testArray, 8);
  for ( int n = 0; n < 8; n++ ) {
    printf("testArray[%i]: %i\n", n, testArray[n]);
  }
}
