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
  (void)public_argc; // all args are stack-passed on i386 SysV

  const size_t ps   = page_size();
  unsigned char *mem = (unsigned char *)mmap(
      NULL, ps, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) return NULL;

  unsigned char *c = mem;

  // prologue: save original return in ECX, insert context, then CALL target
  *c++ = 0x59;                          // pop ecx                 ; save caller_ret
  *c++ = 0x68;                          // push imm32 (context)
  memcpy(c, &context, 4); c += 4;

  // call rel32 target_func
  {
    *c++ = 0xE8;                        // call rel32
    int32_t rel = (int32_t)((intptr_t)target_func - ((intptr_t)c + 4));
    memcpy(c, &rel, 4); c += 4;
  }

  // epilogue: drop injected context, restore caller_ret, return
  *c++ = 0x83; *c++ = 0xC4; *c++ = 0x04; // add esp, 4             ; pop context
  *c++ = 0x51;                          // push ecx                ; push caller_ret
  *c++ = 0xC3;                          // ret

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
