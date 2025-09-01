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

  // For x86 cdecl: all args on stack, we need to insert context as first arg
  // Stack on entry: [ret_addr][arg0][arg1]...
  // We need to call target with: [ret_addr][context][arg0][arg1]...
  
  if (public_argc == 0) {
    // No arguments to preserve, just push context
    *c++ = 0x68;                          // push imm32
    memcpy(c, &context, 4); c += 4;
  } else {
    // We have arguments to preserve
    // The trick: we'll use a call instruction which will push a new return address
    // Then inside the target, it will return to our cleanup code
    
    // Push context as first argument
    *c++ = 0x68;                          // push imm32
    memcpy(c, &context, 4); c += 4;
    
    // Call the target function (this pushes return address)
    *c++ = 0xE8;                          // call rel32
    int32_t rel = (int32_t)((intptr_t)target_func - ((intptr_t)(c + 4)));
    memcpy(c, &rel, 4); c += 4;
    
    // After target returns, clean up the context we pushed
    *c++ = 0x83; *c++ = 0xC4; *c++ = 0x04;  // add esp, 4
    
    // Return to original caller
    *c++ = 0xC3;                          // ret
    
    // Make sure we don't fall through to the code below
    if (mprotect(mem, ps, PROT_READ | PROT_EXEC) != 0) {
      munmap(mem, ps);
      return NULL;
    }
    return mem;
  }
  
  // Jump to target (for no-arg case)
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
