#include <stdio.h>
#include <stdlib.h>
#include <exec/types.h>

#include "pair.h"

int main(int argc, char **argv) {
  const char* left = "Left side";
  const char* right = "Right side";
  
  Pair* pair = PairFrom((APTR)left, (APTR)right);
  
  printf("Left: %s\n", (STRPTR)pair->left());
  printf("Right: %s\n", (STRPTR)pair->right());
  
  pair->free();
  
  return 0;
}