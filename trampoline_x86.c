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

  // For x86 cdecl: we need to prepend context to the argument list
  // Stack on entry: [ret_addr][arg0][arg1]...
  // We want:        [ret_addr][context][arg0][arg1]...
  
  // Method: use the standard function prologue and manipulate arguments
  *c++ = 0x55;                          // push ebp
  *c++ = 0x89; *c++ = 0xE5;            // mov ebp, esp
  
  // Now stack is: [saved_ebp][ret_addr][arg0][arg1]...
  // ebp+4 = return address
  // ebp+8 = first arg
  // ebp+12 = second arg, etc.
  
  // Push all original arguments in reverse order (rightmost first)
  for (int i = public_argc - 1; i >= 0; i--) {
    *c++ = 0xFF; *c++ = 0x75; *c++ = (unsigned char)(8 + i * 4);  // push [ebp + 8 + i*4]
  }
  
  // Push context as first argument
  *c++ = 0x68;                          // push imm32
  memcpy(c, &context, 4); c += 4;
  
  // Call target function
  *c++ = 0xE8;                          // call rel32
  int32_t rel = (int32_t)((intptr_t)target_func - ((intptr_t)(c + 4)));
  memcpy(c, &rel, 4); c += 4;
  
  // Clean up stack (context + original args)
  unsigned char cleanup = (public_argc + 1) * 4;
  *c++ = 0x83; *c++ = 0xC4; *c++ = cleanup;  // add esp, cleanup
  
  // Standard function epilogue
  *c++ = 0x5D;                          // pop ebp
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
