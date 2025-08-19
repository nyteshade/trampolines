// Target: ARM64 (aarch64) | ABI: AAPCS64 (Apple, Linux) | Arg1: x0
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define TRAMPOLINE_SIZE 40

static uint32_t* aarch64_emit_load_imm64(uint32_t* buffer, uint8_t reg, uint64_t value) {
    buffer[0] = (0xd2800000) | ((uint32_t)(value & 0xFFFF) << 5) | reg;
    buffer[1] = (0xf2a00000) | ((uint32_t)((value >> 16) & 0xFFFF) << 5) | reg;
    buffer[2] = (0xf2c00000) | ((uint32_t)((value >> 32) & 0xFFFF) << 5) | reg;
    buffer[3] = (0xf2e00000) | ((uint32_t)((value >> 48) & 0xFFFF) << 5) | reg;
    return buffer + 4;
}

void *trampoline_create(void *target_func, void *context) {
    void *mem = mmap(NULL, TRAMPOLINE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) return NULL;

    uint32_t *code = (uint32_t *)mem;
    code = aarch64_emit_load_imm64(code, 16, (uint64_t)context);
    code = aarch64_emit_load_imm64(code, 17, (uint64_t)target_func);
    *code++ = 0xaa1003e0; // mov x0, x16
    *code++ = 0xd61f0220; // br x17

    if (mprotect(mem, TRAMPOLINE_SIZE, PROT_READ | PROT_EXEC) == -1) {
        munmap(mem, TRAMPOLINE_SIZE);
        return NULL;
    }
    __builtin___clear_cache((char*)mem, (char*)code);
    return mem;
}

void trampoline_free(void *trampoline) {
    if (trampoline) munmap(trampoline, TRAMPOLINE_SIZE);
}
