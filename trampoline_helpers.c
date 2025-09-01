#include "trampoline.h"

#ifdef HAS_PUBLIC_ARGC
void *trampoline_create_and_track(
  void* target_func, 
  void* context, 
  size_t public_args,
  trampoline_allocations* allocations
) {
  void *pointer = trampoline_create(target_func, context, public_args);
  
  if (allocations) {
    if (pointer) {
      allocations->pointers[allocations->next++] = pointer;
    }
    else {
      allocations->failures++;
    }
  }
  
  return pointer;
}

#else

void *trampoline_create_and_track(
  void* target_func, 
  void* context, 
  trampoline_allocations* allocations
) {
  void *pointer = trampoline_create(target_func, context);
  
  if (allocations) {
    if (pointer) {
      allocations->pointers[allocations->next++] = pointer;
    }
    else {
      allocations->failures++;
    }
  }
  
  return pointer;
}

#endif

unsigned char trampolines_validate(
  trampoline_allocations *allocations
) {
  unsigned int i = 0;
  
  if (allocations) {
    if (allocations->failures) {
      for (i = 0; i < allocations->next; i++) {
        trampoline_free(allocations->pointers[i]);
      }
    
      return 0;
    }
  }
  else return 0;
  
  return 1;
}
