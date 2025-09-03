#include <trampoline.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "array_example.h"
#include "array_example_impl.c"

int main(int argc, char **argv) {
  PtrArray* array = PtrArrayMake(2);

  printf("Fresh array (%lu size, %lu capacity)\n", array->size(), array->capacity());

  array->append("Hello world");
  printf("Added element (%lu size, %lu capacity)\n", array->size(), array->capacity());

  array->append("Goodbye cruel world");
  printf("Added element (%lu size, %lu capacity)\n", array->size(), array->capacity());

  array->append("Laters");
  printf("Added element (%lu size, %lu capacity)\n", array->size(), array->capacity());

  printf("Last element is %s\n", (char*)array->last());
  printf("Element at index 1 is %s\n", (char*)array->elementAt(1));
  printf("First element is %s\n", (char*)array->first());

  array->free();

  return 0;
}
