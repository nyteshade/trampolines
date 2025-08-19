// Target: x86-64
// ABI: System V AMD64 (Linux, macOS)
// First argument passed in register: rdi

#include "trampoline.h"
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

static const size_t TRAMPOLINE_SIZE = 22; // 10 bytes for mov + 12 for jmp

void *trampoline_create(void *target_func, void *context) {
    void *mem = mmap(NULL, TRAMPOLINE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) return NULL;

    unsigned char *code = (unsigned char *)mem;

    // movabs rdi, <64-bit context>
    *code++ = 0x48; *code++ = 0xbf;
    memcpy(code, &context, sizeof(uint64_t));
    code += sizeof(uint64_t);

    // movabs rax, <64-bit function>
    *code++ = 0x48; *code++ = 0xb8;
    memcpy(code, &target_func, sizeof(uint64_t));
    code += sizeof(uint64_t);

    // jmp rax
    *code++ = 0xff; *code++ = 0xe0;

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
