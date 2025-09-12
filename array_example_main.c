#include "array_example.h"
#include <stdio.h>

int main(int argc, char **argv) {
  PtrArray* array = PtrArrayMake(2);
  int i;

  if (!array) {
    printf("Failed to create trampoline struct\n");
    return 1;
  }

  printf("Adding initial value (capacity %ld/%ld)\n", array->size(), array->capacity());
  array->append("Initial");

  for (i = 1; i < argc; i++) {
    printf(
      "Adding specified value %s (capacity %ld/%ld)\n",
      argv[i],
      array->size(),
      array->capacity()
    );

    array->append(argv[i]);
  }

  printf("The items in the array are:\n");
  for (i = 0; i < array->size(); i++) {
    printf("\t%d - %s\n", i, (char*)array->elementAt(i));
  }

  printf("\nFreeing array\n");
  array->free();

  return 0;
}
