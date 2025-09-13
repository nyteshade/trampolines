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

  // For i386 cdecl calling convention:
  // All arguments are passed on the stack
  // Stack on entry: [ret_addr][arg0][arg1]...
  // We need:        [ret_addr][context][arg0][arg1]...
  
  // For i386 cdecl calling convention, we need to:
  // 1. Push context as the first argument 
  // 2. CALL the target function (not JMP, so it has a return address)
  // 3. Clean up our added argument
  // 4. Return to the original caller
  //
  // This works because:
  // - Original caller pushes args and calls trampoline
  // - Trampoline pushes context and calls target  
  // - Target returns to trampoline
  // - Trampoline cleans up and returns to original caller

  if (public_argc == 0) {
    // Simple case: no original arguments
    *c++ = 0x68;                        // push imm32 (context)
    memcpy(c, &context, 4); c += 4;
    
    *c++ = 0xE8;                        // call rel32
    int32_t rel = (int32_t)((intptr_t)target_func - ((intptr_t)c + 4));
    memcpy(c, &rel, 4); c += 4;
    
    *c++ = 0x83; *c++ = 0xC4; *c++ = 0x04;  // add esp, 4 (clean up context)
    *c++ = 0xC3;                        // ret
  } else {
    // With arguments: need to push context + original args in cdecl order
    // Original stack: [ret][arg0][arg1]...
    // Target expects: [ret][context][arg0][arg1]...
    // 
    // For cdecl: args pushed right-to-left, so:
    // 1. Push rightmost args first: arg1, arg0 
    // 2. Push context last (leftmost)
    
    // Push original arguments in reverse order (rightmost first)
    // For cdecl: args pushed right-to-left, so push arg[n-1], arg[n-2], ..., arg[0]
    // As we push each arg, ESP decreases by 4, so subsequent offsets must account for this
    // 
    // Initial stack: [ret_addr][arg0][arg1]...[arg(n-1)]
    // We want to push in order: arg(n-1), arg(n-2), ..., arg0
    // After pushing k args, ESP has decreased by k*4
    // So when pushing arg[i], the original arg[i] is now at [esp + 4 + i*4 + k*4]
    // where k is the number of args already pushed
    
    size_t i;
    for (i = 0; i < public_argc; i++) {
      // Push argument at index (public_argc - 1 - i), which is the (i+1)th from the right
      // Number of args already pushed = i
      // Original arg[public_argc - 1 - i] is at offset: 4 + (public_argc - 1 - i)*4 + i*4
      //   = 4 + public_argc*4 - 4 - i*4 + i*4 = public_argc*4
      uint8_t offset = (uint8_t)(4 + (public_argc - 1 - i) * 4 + i * 4);
      *c++ = 0xFF; *c++ = 0x74; *c++ = 0x24; *c++ = offset;  // push [esp+offset]
    }
    
    // Then push context (leftmost argument)
    *c++ = 0x68;                        // push imm32 (context)
    memcpy(c, &context, 4); c += 4;
    
    // Call target function
    *c++ = 0xE8;                        // call rel32
    int32_t rel = (int32_t)((intptr_t)target_func - ((intptr_t)c + 4));
    memcpy(c, &rel, 4); c += 4;
    
    // Clean up: remove context + all duplicated args
    uint32_t cleanup_bytes = (public_argc + 1) * 4;
    if (cleanup_bytes <= 127) {
      *c++ = 0x83; *c++ = 0xC4; *c++ = (uint8_t)cleanup_bytes;  // add esp, imm8
    } else {
      *c++ = 0x81; *c++ = 0xC4;                                  // add esp, imm32
      memcpy(c, &cleanup_bytes, 4); c += 4;
    }
    
    *c++ = 0xC3;                        // ret
  }

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
