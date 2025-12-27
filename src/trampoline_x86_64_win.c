// Target: x86-64 Microsoft x64 (Windows)
// Windows x64 ABI: RCX, RDX, R8, R9 for first 4 args, rest on stack
// We need to shift args right and insert context as first arg (RCX)
#include "trampoline.h"
#include <windows.h>
#include <stdint.h>
#include <string.h>

enum { TRAMP_MAX = 256 };

void *trampoline_create(void *target_func, void *context, size_t public_argc) {
    uint8_t *mem = (uint8_t*)VirtualAlloc(NULL, TRAMP_MAX, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!mem) return NULL;
    uint8_t *p = mem;

    // Windows x64 ABI: shadow space at [rsp+8], [rsp+16], [rsp+24], [rsp+32]
    // Stack args start at [rsp+40] (0x28)
    // We need to shift all args right by one position
    
    // Save r11 to use as temp register if we need it
    if (public_argc >= 4) {
        // Save original r9 in r11 before we shift
        *p++ = 0x4D; *p++ = 0x89; *p++ = 0xCB;  // mov r11, r9
    }
    
    // Shift register arguments: rcx->rdx, rdx->r8, r8->r9
    // Do it backwards to avoid clobbering
    if (public_argc >= 3) {
        *p++ = 0x4D; *p++ = 0x89; *p++ = 0xC1;  // mov r9, r8
    }
    if (public_argc >= 2) {
        *p++ = 0x49; *p++ = 0x89; *p++ = 0xD0;  // mov r8, rdx
    }
    if (public_argc >= 1) {
        *p++ = 0x48; *p++ = 0x89; *p++ = 0xCA;  // mov rdx, rcx
    }
    
    // If we have 4+ args, we need to handle stack arguments
    if (public_argc >= 4) {
        // The old 4th arg (was in r9, now in r11) goes to stack at [rsp+0x28]
        *p++ = 0x4C; *p++ = 0x89; *p++ = 0x5C; *p++ = 0x24; *p++ = 0x28;  // mov [rsp+0x28], r11
        
        // If we have 5+ args, shift all stack args right by 8 bytes
        if (public_argc >= 5) {
            // We need to shift stack args from [rsp+0x28] onwards
            // Do it from the end to avoid overwriting
            size_t stack_args = public_argc - 4;
            
            // Use r10 and r11 as scratch registers
            for (int i = stack_args - 1; i >= 0; i--) {
                uint32_t src_offset = 0x28 + (i * 8);
                uint32_t dst_offset = 0x30 + (i * 8);  // One slot higher
                
                // mov r10, [rsp+src_offset]
                *p++ = 0x4C; *p++ = 0x8B; *p++ = 0x54; *p++ = 0x24; *p++ = src_offset;
                // mov [rsp+dst_offset], r10
                *p++ = 0x4C; *p++ = 0x89; *p++ = 0x54; *p++ = 0x24; *p++ = dst_offset;
            }
        }
    }
    
    // Load context into rcx (first argument)
    // movabs rcx, context
    *p++ = 0x48; *p++ = 0xB9;
    memcpy(p, &context, 8); p += 8;
    
    // Load target function address and jump to it
    // movabs rax, target_func
    *p++ = 0x48; *p++ = 0xB8;
    memcpy(p, &target_func, 8); p += 8;
    
    // jmp rax
    *p++ = 0xFF; *p++ = 0xE0;

    // Make memory executable
    DWORD old;
    if (!VirtualProtect(mem, TRAMP_MAX, PAGE_EXECUTE_READ, &old)) {
        VirtualFree(mem, 0, MEM_RELEASE);
        return NULL;
    }
    FlushInstructionCache(GetCurrentProcess(), mem, TRAMP_MAX);
    return mem;
}

void trampoline_free(void *tramp) {
    if (tramp) VirtualFree(tramp, 0, MEM_RELEASE);
}