// Target: x86 (i386) POSIX SysV cdecl (Linux, macOS 10.6–10.8)
// Prepends `context` and calls `target_func(self, A1, …)`;
// then removes the injected context and returns to the original caller.

#include "trampoline.h"
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

static size_t page_size(void) {
  long ps = sysconf(_SC_PAGESIZE);
  return (ps > 0) ? (size_t)ps : 4096u;
}

void *trampoline_create(void *target_func, void *context, size_t public_argc) {
  const size_t ps   = page_size();
  unsigned char *mem = (unsigned char *)mmap(
      NULL, ps, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) return NULL;

  unsigned char *c = mem;

  // For i386 cdecl: we need to insert context as first argument
  // Stack on entry: [ret_addr][arg0][arg1]...
  // Stack needed:   [ret_addr][context][arg0][arg1]...
  
  if (public_argc == 0) {
    // No arguments - simple case, just push context and jump
    *c++ = 0x68;                        // push imm32
    memcpy(c, &context, 4); c += 4;
  } else {
    // We have arguments - need to make room for context
    // Strategy: pop return address, push context, push return address, then jump
    
    *c++ = 0x58;                        // pop eax ; return address in eax
    *c++ = 0x68;                        // push imm32 ; push context
    memcpy(c, &context, 4); c += 4;
    *c++ = 0x50;                        // push eax ; push return address back
  }
  
  // Jump to target function
  *c++ = 0xE9;                          // jmp rel32
  int32_t rel = (int32_t)((intptr_t)target_func - ((intptr_t)c + 4));
  memcpy(c, &rel, 4); c += 4;

  // RX permissions
  if (mprotect(mem, ps, PROT_READ | PROT_EXEC) != 0) {
    munmap(mem, ps);
    return NULL;
  }
  return mem;
}

void trampoline_free(void *trampoline) {
  if (!trampoline) return;
  munmap((void *)((uintptr_t)trampoline & ~((uintptr_t)page_size() - 1)), page_size());
}
