#ifndef ARRAY_EXAMPLE_IMPL_C
#define ARRAY_EXAMPLE_IMPL_C

#include <trampoline.h>
#include <stdlib.h>
#include <stddef.h>

#include "array_example.h"
#include "trampoline.h"

typedef struct PtrArray_ {
  PtrArray public;

  void**  items;
  size_t  size;
  size_t  capacity;
} PtrArray_;

TI_Getter(ptrarray_capacity, PtrArray, PtrArray_, size_t, capacity);
TI_Getter(ptrarray_size, PtrArray, PtrArray_, size_t, size);

TF_Unary(void*, ptrarray_elementAt, PtrArray, PtrArray_, size_t, index)
  void* element = NULL;

  if (self && index <= private->size) {
    element = private->items[index];
  }

  return element;
}

TF_Getter(ptrarray_first, PtrArray, PtrArray_, void*)
  void* element = NULL;

  if (self && 1 <= private->size) {
    element = private->items[0];
  }

  return element;
}

TF_Getter(ptrarray_last, PtrArray, PtrArray_, void*)
  void* element = NULL;

  if (self && private->size >= 1) {
    element = private->items[private->size - 1];
  }

  return element;
}

TF_Unary(void, ptrarray_append, PtrArray, PtrArray_, void*, element)
  void** bigger = NULL;

  if (self) {
    if ((private->size + 1) >= private->capacity) {
      bigger = (void**)realloc(private->items, (private->capacity + 10) * sizeof(void*));

      if (bigger) {
        private->items = bigger;
        private->capacity += 10;
      }
      /* else, create new and copy */
    }

    private->items[private->size++] = element;
  }
}

TF_Nullary(ptrarray_free, PtrArray, PtrArray_)
  if (private) {
    free(private->items);
    trampoline_tracker_free_by_context(self);
    free(private);
  }
}

PtrArray* PtrArrayMake(size_t initial_capacity) {
  TA_Allocate(PtrArray, PtrArray_);

  if (private) {
    private->items = (void**)calloc(initial_capacity, sizeof(void*));
    private->capacity = initial_capacity;
    private->size = 0;

    public->capacity = trampoline_monitor(ptrarray_capacity, public, 0, &tracker);
    public->size = trampoline_monitor(ptrarray_size, public, 0, &tracker);

    public->elementAt = trampoline_monitor(ptrarray_elementAt, public, 1, &tracker);
    public->first = trampoline_monitor(ptrarray_first, public, 0, &tracker);
    public->last = trampoline_monitor(ptrarray_last, public, 0, &tracker);

    public->append = trampoline_monitor(ptrarray_append, public, 1, &tracker);

    public->free = trampoline_monitor(ptrarray_free, public, 0, &tracker);

    if (!trampoline_validate(tracker)) {
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
