// Target: x86 (i386)
// ABI: cdecl (Linux, macOS)
// First argument passed on the stack

#include "trampoline.h"
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

static const size_t TRAMPOLINE_SIZE = 12;

void *trampoline_create(void *target_func, void *context) {
    void *mem = mmap(NULL, TRAMPOLINE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) return NULL;

    unsigned char *code = (unsigned char *)mem;

    // mov eax, <32-bit context>
    *code++ = 0xb8;
    memcpy(code, &context, sizeof(uint32_t));
    code += sizeof(uint32_t);

    // push eax
    *code++ = 0x50;

    // call <relative_offset_to_function>
    *code++ = 0xe8;
    int32_t offset = (intptr_t)target_func - ((intptr_t)code + 4);
    memcpy(code, &offset, sizeof(int32_t));
    code += sizeof(int32_t);

    // add esp, 4 (cleanup stack)
    *code++ = 0x83; *code++ = 0xc4; *code++ = 0x04;

    // ret (not strictly needed after call, but good form)
    *code++ = 0xc3;

    if (mprotect(mem, TRAMPOLINE_SIZE, PROT_READ | PROT_EXEC) == -1) {
        munmap(mem, TRAMPOLINE_SIZE);
        return NULL;
    }
    return mem;
}

void trampoline_free(void *trampoline) {
    if (trampoline) {
        munmap(trampoline, TRAMPOLINE_SIZE);
    }
}
