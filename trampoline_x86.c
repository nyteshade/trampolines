// Target: x86 (i386) POSIX cdecl
// Works for any n: pop ret, push context, push ret, jmp target.

#include "trampoline.h"
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static const size_t SZ = 1+5+1+5; // pop + push imm32 + push ecx + jmp rel32

void *trampoline_create(void *target_func, void *context, size_t public_argc) {
  (void)public_argc;
  unsigned char *mem = mmap(NULL, SZ, PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) return NULL;
  unsigned char *c = mem;

  *c++ = 0x59;                 // pop ecx        (ret_to_caller)
  *c++ = 0x68; memcpy(c,&context,4); c+=4; // push imm32 (context)
  *c++ = 0x51;                 // push ecx       (ret_to_caller)
  // jmp rel32 target
  *c++ = 0xE9; int32_t rel = (int32_t)((intptr_t)target_func - ((intptr_t)c + 4));
  memcpy(c, &rel, 4); c+=4;

  mprotect(mem, SZ, PROT_READ|PROT_EXEC);
  return mem;
}

void trampoline_free(void *trampoline) {
  if (trampoline) munmap(trampoline, SZ);
}
