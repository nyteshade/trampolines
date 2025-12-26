#include <trampoline.h>
#include <stdlib.h>
#include <stddef.h>

#include <exec/exec.h>

#include "pair.h"

typedef struct Pair_ {
  Pair pair;
  
  APTR left;
  APTR right;
} Pair_;

TI_Property(pair_get_left, pair_set_left, Pair, Pair_, APTR, left)
TI_Property(pair_get_right, pair_set_right, Pair, Pair_, APTR, right)

TF_Nullary(pair_free, Pair, Pair_)
  if (private) 
    free(private);
    
  // do not attempt to also free self as it is included in private
}

Pair* PairMake() {  
  TA_Allocate(Pair, Pair_);
  
  if (private) {
    private->left = NULL;
    private->right = NULL;

    TAProperty(left, setLeft, pair_get_left, pair_set_left);
    TAProperty(right, setRight, pair_get_right, pair_set_right);
    TAFunction(free, pair_free);
    
    if (!trampoline_validate(tracker)) {
      free(private);
      public = NULL;
    }
  }
  
  return public;
}

Pair* PairFrom(APTR left, APTR right) {
  Pair* pair = PairMake();
  
  if (pair) {
    pair->setLeft(left);
    pair->setRight(right);
  }
  
  return pair;
}