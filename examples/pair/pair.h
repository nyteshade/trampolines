#ifndef TRAMPOLINE_PAIR_H
#define TRAMPOLINE_PAIR_H

#include <trampoline.h>
#include <exec/types.h>

typedef struct Pair {
  TDProperty(left, setLeft, APTR);
  TDProperty(right, setRight, APTR);
  TDVoidFunc(free);
} Pair;

Pair* PairMake();
Pair* PairFrom(APTR left, APTR right);

#endif