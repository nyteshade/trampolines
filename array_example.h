#ifndef PTR_ARRAY_EXAMPLE_H
#define PTR_ARRAY_EXAMPLE_H

#include <trampoline.h>
#include <stddef.h>

typedef struct PtrArray {
  TRGetter(capacity, size_t);       /* same as `size_t (*capacity)();` */
  TRGetter(size, size_t);           /* same as `size_t (*size)();` */

  TRGetter(first, void*);           /* same as `void* (*first)();` */
  TRGetter(last, void*);            /* same as `void* (*last)();` */

  TRUnary(void, append, void*);     /* same as `void (*append)(void*); */
  TRUnary(void*, elementAt, int);   /* same as `void* (*elementAt)(int); */

  TRNullary(free);                  /* same as `void (*free)();` */
} PtrArray;

PtrArray* PtrArrayMake(size_t initial_capacity);

#endif
