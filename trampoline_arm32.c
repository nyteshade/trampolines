// Target: ARM32 (armv7)
// ABI: AAPCS
// First argument passed in register: r0

#include "trampoline.h"
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>

static const size_t TRAMPOLINE_SIZE = 16; // 4 instructions + 2 data words

void *trampoline_create(void *target_func, void *context) {
    void *mem = mmap(NULL, TRAMPOLINE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) return NULL;

    uint32_t *code = (uint32_t *)mem;

    // Instructions
    code[0] = 0xe59f0004; // ldr r0, [pc, #4]  (load context)
    code[1] = 0xe59f1004; // ldr r1, [pc, #4]  (load function)
    code[2] = 0xe12fff11; // bx r1             (branch to function)
    code[3] = 0xe1a00000; // nop (alignment)

    // Literal Pool (data)
    code[4] = (uint32_t)context;
    code[5] = (uint32_t)target_func;

    if (mprotect(mem, TRAMPOLINE_SIZE, PROT_READ | PROT_EXEC) == -1) {
        munmap(mem, TRAMPOLINE_SIZE);
        return NULL;
    }

    __builtin___clear_cache((char*)mem, (char*)mem + TRAMPOLINE_SIZE);
    return mem;
}

void trampoline_free(void *trampoline) {
    if (trampoline) {
        munmap(trampoline, TRAMPOLINE_SIZE);
    }
}
