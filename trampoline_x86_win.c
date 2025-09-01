// Target: x86 (i386) Windows cdecl
// Windows uses cdecl for C functions, same as Unix but with different memory APIs
#include "trampoline.h"
#include <windows.h>
#include <stdint.h>
#include <string.h>

static const size_t TRAMP_SIZE = 64; // Enough for our code

void *trampoline_create(void *target_func, void *context, size_t public_argc) {
    unsigned char *mem = (unsigned char*)VirtualAlloc(
        NULL, TRAMP_SIZE, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!mem) return NULL;
    
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
    *c++ = 0xE9;                            // jmp rel32
    int32_t rel = (int32_t)((intptr_t)target_func - ((intptr_t)c + 4));
    memcpy(c, &rel, 4); c += 4;

    // Make memory executable
    DWORD old;
    if (!VirtualProtect(mem, TRAMP_SIZE, PAGE_EXECUTE_READ, &old)) {
        VirtualFree(mem, 0, MEM_RELEASE);
        return NULL;
    }
    FlushInstructionCache(GetCurrentProcess(), mem, TRAMP_SIZE);
    return mem;
}

void trampoline_free(void *trampoline) {
    if (trampoline) VirtualFree(trampoline, 0, MEM_RELEASE);
}
  