// Target: x86 (i386) Windows cdecl
#include "trampoline.h"
#include <windows.h>
#include <stdint.h>
#include <string.h>

static const size_t SZ = 1+5+1+5;

void *trampoline_create(void *target_func, void *context, size_t public_argc) {
    (void)public_argc;
    unsigned char *mem = (unsigned char*)VirtualAlloc(NULL, SZ, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!mem) return NULL;
    unsigned char *c = mem;

    *c++ = 0x59;                 // pop ecx
    *c++ = 0x68; memcpy(c,&context,4); c+=4; // push imm32
    *c++ = 0x51;                 // push ecx
    *c++ = 0xE9; INT32 rel = (INT32)((INT_PTR)target_func - ((INT_PTR)c + 4));
    memcpy(c, &rel, 4); c+=4;

    DWORD old; VirtualProtect(mem, SZ, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), mem, SZ);
    return mem;
  }

  void trampoline_free(void *trampoline) {
    if (trampoline) VirtualFree(trampoline, 0, MEM_RELEASE);
  }
  