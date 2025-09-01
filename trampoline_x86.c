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

  // For x86 cdecl: all args on stack, we need to shift them and insert context
  // Stack on entry: [ret_addr][arg0][arg1]...
  // We need:        [ret_addr][context][arg0][arg1]...
  
  // Calculate how many bytes to copy (args + return address)
  // We need to shift: public_argc * 4 bytes of arguments
  size_t bytes_to_shift = public_argc * 4;
  
  if (bytes_to_shift > 0) {
    // Save registers we'll use
    *c++ = 0x50;                        // push eax
    *c++ = 0x51;                        // push ecx  
    *c++ = 0x52;                        // push edx
    *c++ = 0x56;                        // push esi
    *c++ = 0x57;                        // push edi
    
    // ESI = source (esp + 28 = skip 5 saved regs (20) + ret addr (4) + point to first arg)
    // EDI = dest (esp + 24 = skip 5 saved regs (20) + ret addr (4))  
    *c++ = 0x8D; *c++ = 0x74; *c++ = 0x24; *c++ = 0x1C;  // lea esi, [esp+28]
    *c++ = 0x8D; *c++ = 0x7C; *c++ = 0x24; *c++ = 0x18;  // lea edi, [esp+24]
    
    // ECX = bytes_to_shift
    *c++ = 0xB9;                        // mov ecx, imm32
    memcpy(c, &bytes_to_shift, 4); c += 4;
    
    // Copy arguments backwards to make room
    *c++ = 0x01; *c++ = 0xCE;           // add esi, ecx  ; point to end
    *c++ = 0x01; *c++ = 0xCF;           // add edi, ecx  ; point to end
    *c++ = 0xFD;                        // std           ; direction flag = backwards
    *c++ = 0xF3; *c++ = 0xA4;           // rep movsb     ; copy ECX bytes
    *c++ = 0xFC;                        // cld           ; clear direction flag
    
    // Store context at [esp+28] (where first arg was before shifting)
    *c++ = 0xC7; *c++ = 0x44; *c++ = 0x24; *c++ = 0x1C;  // mov dword [esp+28], imm32
    memcpy(c, &context, 4); c += 4;
    
    // Restore registers
    *c++ = 0x5F;                        // pop edi
    *c++ = 0x5E;                        // pop esi
    *c++ = 0x5A;                        // pop edx
    *c++ = 0x59;                        // pop ecx
    *c++ = 0x58;                        // pop eax
  } else {
    // No args to shift, just push context
    *c++ = 0x68;                        // push imm32
    memcpy(c, &context, 4); c += 4;
  }
  
  // Jump to target (not call, to preserve stack layout)
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
