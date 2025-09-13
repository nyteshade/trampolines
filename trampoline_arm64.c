// Target: ARM64 (AAPCS64)
// Shift x0..x7 right by 1 as needed, x0=context.
// If public_argc >= 8, keep 16B alignment and push old x7 as new first stack arg.
// Use blr + ret so we can restore sp when we pushed.

#include "trampoline.h"
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define EMIT32(x)  (*code++ = (uint32_t)(x))
static uint32_t* mov_imm64(uint32_t* b, uint8_t xd, uint64_t v) {
    *b++ = 0xD2800000 | ((v & 0xFFFF) << 5)  | xd;          // movz
    *b++ = 0xF2A00000 | (((v>>16)&0xFFFF) << 5) | xd;       // movk #16
    *b++ = 0xF2C00000 | (((v>>32)&0xFFFF) << 5) | xd;       // movk #32
    *b++ = 0xF2E00000 | (((v>>48)&0xFFFF) << 5) | xd;       // movk #48
    return b;
  }
  static inline uint32_t mov_reg(uint8_t xd, uint8_t xm) {   // mov xd,xm
    return 0xAA0003E0 | (xm<<16) | xd;
  }
  __attribute__((unused)) static inline uint32_t add_imm_sp(uint8_t xd, uint32_t imm12) { // add xd,sp,#imm
    return 0x91000000 | (imm12<<10) | (31<<5) | xd; // 31 encodes SP
  }
  static inline uint32_t sub_imm_sp(uint32_t imm12) { // sub sp,sp,#imm
    return 0xD1000000 | (imm12<<10) | (31<<5) | 31;
  }

  enum { SIZE = 128 };

  void *trampoline_create(void *target_func, void *context, size_t public_argc) {
    void *mem = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) return NULL;

    uint32_t *code = (uint32_t*)mem;

    // Load context->x16, target->x17
    code = mov_imm64(code, 16, (uint64_t)context);
    code = mov_imm64(code, 17, (uint64_t)target_func);

    // Save old x7 in x9 (harmless if argc<7)
    EMIT32(mov_reg(9, 7));

    // Shift regs: for i=min(7,argc)..1: mov x{i}, x{i-1}
    size_t maxr = public_argc < 7 ? public_argc : 7;
    for (size_t i = maxr; i >= 1; i--) {
      EMIT32(mov_reg((uint8_t)i, (uint8_t)(i-1)));
      if (i == 1) break;
    }

    // x0 = context
    EMIT32(mov_reg(0, 16));

    if (public_argc >= 8) {
      // Keep 16B alignment, push old x7 into [sp], pad 8 bytes
      // sub sp, sp, #16
      EMIT32(sub_imm_sp(16)); // imm12 is unscaled for sub/add
      // str x9, [sp]
      EMIT32(0xF90003E9); // STR X9, [SP,#0]
      // str xzr, [sp,#8]   (pad)
      EMIT32(0xF90007FF); // STR XZR,[SP,#8]
      // blr x17
      EMIT32(0xD63F0220);
      // add sp, sp, #16 ; ret
      EMIT32(0x910043FF); // ADD SP,SP,#16 - direct encoding
      // ret
      EMIT32(0xD65F03C0);
    } else {
      // mov x0 done; just branch to target and return to caller.
      // br x17
      EMIT32(0xD61F0220);
    }

    __builtin___clear_cache((char*)mem, (char*)mem + SIZE);
    mprotect(mem, SIZE, PROT_READ|PROT_EXEC);
    return mem;
  }

  void trampoline_free(void *trampoline) {
    if (trampoline) munmap(trampoline, SIZE);
  }
  