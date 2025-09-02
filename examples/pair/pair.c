#include <trampoline.h>
#include <stddef.h>

#include <exec/exec.h>

#include "pair.h"

typedef struct Pair_ {
  Pair pair;
  
  APTR left;
  APTR right;
  
  trampoline_allocations allocations;
} Pair_;

TRAMP_PROPERTY_(pair_left, Pair, Pair_, APTR, left);
TRAMP_PROPERTY_(pair_right, Pair, Pair_, APTR, right);

void pair_free(Pair* self) {
  Pair_* private = (Pair_*)self;
  int i = 0;
  
  for (i = 0; i < private->allocations.next; i++) {
    if (private->allocations.pointers[i]) {
      trampoline_free(private->allocations.pointers[i]);
    }
  }
  
  free(private);
}

Pair* PairMake() {
  Pair_* pair_ = (Pair_*)AllocVec(sizeof(Pair_), MEMF_PUBLIC|MEMF_CLEAR);
  Pair* pair = NULL;
  
  if (pair_) {
    pair = (Pair*)pair_;
    
    pair->left = trampoline_create_and_track(
      get_private_pair_left, pair, &pair_->allocations
    );
    
    pair->right = trampoline_create_and_track(
      get_private_pair_right, pair, &pair_->allocations
    );
    
    pair->setLeft = trampoline_create_and_track(
      set_private_pair_left, pair, &pair_->allocations
    );
    
    pair->setRight = trampoline_create_and_track(
      set_private_pair_right, pair, &pair_->allocations
    );
    
    pair->free = trampoline_create_and_track(
      pair_free, pair, &pair_->allocations
    );

    return (Pair*)pair_;
  }
 
  return NULL;
}

Pair* PairFrom(APTR left, APTR right) {
  Pair* pair = PairMake();
  
  if (pair) {
    pair->setLeft(left);
    pair->setRight(right);
  }
  
  return pair;
}