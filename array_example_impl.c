#ifndef ARRAY_EXAMPLE_IMPL_C
#define ARRAY_EXAMPLE_IMPL_C

#include <trampoline.h>
#include <stdlib.h>
#include <stddef.h>

#include "array_example.h"

typedef struct PtrArray_ {
  PtrArray public;
  
  void**  items;
  size_t  size;
  size_t  capacity;
} PtrArray_;

TRAMP_GETTER_(ptrarray_capacity, PtrArray, PtrArray_, size_t, capacity);
TRAMP_GETTER_(ptrarray_size, PtrArray, PtrArray_, size_t, size);

void* ptrarray_elementAt(PtrArray* self, size_t index) {
  PtrArray_* private = (PtrArray_*)self;
  void* element = NULL;
  
  if (self && index <= private->size) {
    element = private->items[index];
  }
  
  return element;
}

void* ptrarray_first(PtrArray* self) {
  PtrArray_* private = (PtrArray_*)self;
  void* element = NULL;
  
  if (self && 1 <= private->size) {
    element = private->items[0];
  }
  
  return element;
}

void* ptrarray_last(PtrArray* self) {
  PtrArray_* private = (PtrArray_*)self;
  void* element = NULL;
  
  if (self && private->size >= 1) {
    element = private->items[private->size - 1];
  }
  
  return element;
}

void ptrarray_append(PtrArray *self, void* element) {
  PtrArray_* private = (PtrArray_*)self;
  void** bigger = NULL;
  
  if (self) {
    if ((private->size + 1) >= private->capacity) {
      bigger = (void**)realloc(private->items, private->capacity + 10);
      
      if (bigger) {
        private->items = bigger;
        private->capacity += 10;
      }
    }
    
    private->items[private->size++] = element;      
  }
}

void ptrarray_free(PtrArray *self) {
  PtrArray_* private = (PtrArray_*)self;
  
  if (private) {
    free(private->items); 
    
    trampoline_free(self->capacity);
    trampoline_free(self->size);
    
    trampoline_free(self->elementAt);
    trampoline_free(self->first);
    trampoline_free(self->last);
    
    trampoline_free(self->append);
    trampoline_free(self->free);
    
    free(private);
  }
}

PtrArray* PtrArrayMake(size_t initial_capacity) {
  PtrArray_* private = (PtrArray_*)calloc(1, sizeof(PtrArray_));
  PtrArray*  public = (PtrArray*)private;
  trampoline_allocations list = { 0 };
  
  if (private) {
    private->items = (void**)calloc(initial_capacity, sizeof(void*));
    private->capacity = initial_capacity;
    private->size = 0;
    
    public->capacity = trampoline_create_and_track(ptrarray_capacity, public, 0, &list);
    public->size = trampoline_create_and_track(ptrarray_size, public, 0, &list);

    public->elementAt = trampoline_create_and_track(ptrarray_elementAt, public, 1, &list);
    public->first = trampoline_create_and_track(ptrarray_first, public, 0, &list);
    public->last = trampoline_create_and_track(ptrarray_last, public, 0, &list);

    public->append = trampoline_create_and_track(ptrarray_append, public, 1, &list);

    public->free = trampoline_create_and_track(ptrarray_free, public, 0, &list);
    
    if (!trampolines_validate(&list)) {
      free(private->items);
      free(private);
    }
  }
  else {
    public = NULL;
  }
  
  return public;
}

#endif