#include <trampoline.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "array_example.h"
#include "array_example_impl.c"

int main(int argc, char **argv) {
  PtrArray* array = PtrArrayMake(2);
  
  array->append("Hello world");
  array->append("Goodbye cruel world");
  array->append("Laters");
  
  printf("Last element is %s\n", array->last());
  printf("First element is %s\n", array->first());
  
  array->free();
  
  return 0;
}