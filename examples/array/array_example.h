#ifndef PTR_ARRAY_EXAMPLE_H
#define PTR_ARRAY_EXAMPLE_H

#include <trampoline/macros.h>
#include <stddef.h>

typedef struct PtrArray {
  TDGetter(capacity, size_t);       /* same as `size_t (*capacity)();` */
  TDGetter(size, size_t);           /* same as `size_t (*size)();` */

  TDGetter(first, void*);           /* same as `void* (*first)();` */
  TDGetter(last, void*);            /* same as `void* (*last)();` */

  TDUnary(void, append, void*);     /* same as `void (*append)(void*); */
  TDUnary(void*, elementAt, int);   /* same as `void* (*elementAt)(int); */

  TDNullary(free);                  /* same as `void (*free)();` */
} PtrArray;

PtrArray* PtrArrayMake(size_t initial_capacity);

#endif
