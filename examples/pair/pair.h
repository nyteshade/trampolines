#ifndef TRAMPOLINE_PAIR_H
#define TRAMPOLINE_PAIR_H

#include <exec/types.h>

typedef struct Pair {
  APTR (*left)();
  APTR (*right)();
  
  void (*setLeft)(APTR left);
  void (*setRight)(APTR right);
  
  void (*free)();
} Pair;

Pair* PairMake();
Pair* PairFrom(APTR left, APTR right);

#endif