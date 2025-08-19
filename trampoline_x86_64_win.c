// Target: x86-64
// ABI: Microsoft x64
// First argument passed in register: rcx

#include "trampoline.h"
#include <windows.h>

static const size_t TRAMPOLINE_SIZE = 22;

void *trampoline_create(void *target_func, void *context) {
    void *mem = VirtualAlloc(NULL, TRAMPOLINE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (mem == NULL) return NULL;

    unsigned char *code = (unsigned char *)mem;

    // movabs rcx, <64-bit context>
    *code++ = 0x48; *code++ = 0xb9;
    memcpy(code, &context, sizeof(UINT64));
    code += sizeof(UINT64);

    // movabs rax, <64-bit function>
    *code++ = 0x48; *code++ = 0xb8;
    memcpy(code, &target_func, sizeof(UINT64));
    code += sizeof(UINT64);

    // jmp rax
    *code++ = 0xff; *code++ = 0xe0;

    DWORD old_protect;
    if (!VirtualProtect(mem, TRAMPOLINE_SIZE, PAGE_EXECUTE_READ, &old_protect)) {
        VirtualFree(mem, 0, MEM_RELEASE);
        return NULL;
    }
    
    FlushInstructionCache(GetCurrentProcess(), mem, TRAMPOLINE_SIZE);
    return mem;
}

void trampoline_free(void *trampoline) {
    if (trampoline) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
    }
}
