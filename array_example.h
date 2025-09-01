#ifndef PTR_ARRAY_EXAMPLE_H
#define PTR_ARRAY_EXAMPLE_H

#include <trampoline.h>
#include <stddef.h>

typedef struct PtrArray {
  size_t (*capacity)();
  size_t (*size)();
  
  void* (*elementAt)(int index);
  void* (*first)();
  void* (*last)();
  
  void  (*append)(void* element);
  
  void  (*free)();
} PtrArray;

PtrArray* PtrArrayMake(size_t initial_capacity);

#endif
