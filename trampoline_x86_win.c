// Target: x86 (i386)
// ABI: cdecl (Windows)
// First argument passed on the stack

#include "trampoline.h"
#include <windows.h>

static const size_t TRAMPOLINE_SIZE = 12;

void *trampoline_create(void *target_func, void *context) {
    void *mem = VirtualAlloc(NULL, TRAMPOLINE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (mem == NULL) return NULL;

    unsigned char *code = (unsigned char *)mem;

    // mov eax, <32-bit context>
    *code++ = 0xb8;
    memcpy(code, &context, sizeof(UINT32));
    code += sizeof(UINT32);

    // push eax
    *code++ = 0x50;

    // call <relative_offset_to_function>
    *code++ = 0xe8;
    INT32 offset = (INT_PTR)target_func - ((INT_PTR)code + 4);
    memcpy(code, &offset, sizeof(INT32));
    code += sizeof(INT32);

    // add esp, 4
    *code++ = 0x83; *code++ = 0xc4; *code++ = 0x04;

    // ret
    *code++ = 0xc3;

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
