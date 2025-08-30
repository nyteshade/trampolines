// Target: ARM32 (AAPCS)
// Shift r0..r3 right by 1; r0=context.
// If public_argc >= 4, keep 8B alignment: push old r3 + pad before call.
// Use blx + bx lr so we can restore sp when we pushed.

#include "trampoline.h"
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

enum { SIZE = 64 };

static inline uint32_t MOV_rr(uint8_t rd, uint8_t rm) { // mov rd, rm
    return 0xE1A00000 | (rd<<12) | rm;
  }
  static inline uint32_t LDR_lit(uint8_t rd, uint8_t pc_rel_bytes) {
    // ldr rd, [pc, #imm]  imm is bytes from this insn+8
    return 0xE59F0000 | (rd<<12) | pc_rel_bytes;
  }
  static inline uint32_t STR_imm(uint8_t rd, uint8_t rn, uint16_t imm) {
    // str rd, [rn, #imm]
    return 0xE5800000 | (rd<<12) | (rn<<16) | imm;
  }
  static inline uint32_t SUB_sp_imm(uint16_t imm) { // sub sp, sp, #imm
    return 0xE24DD000 | imm;
  }
  static inline uint32_t ADD_sp_imm(uint16_t imm) { // add sp, sp, #imm
    return 0xE28DD000 | imm;
  }

  void *trampoline_create(void *target_func, void *context, size_t public_argc) {
    void *mem = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) return NULL;

    uint32_t *c = (uint32_t*)mem;

    // Save old r3 in r12
    *c++ = MOV_rr(12, 3);

    // Shift r3<=r2, r2<=r1, r1<=r0 (top-down to avoid clobber)
    if (public_argc >= 3) *c++ = MOV_rr(3, 2);
    if (public_argc >= 2) *c++ = MOV_rr(2, 1);
    if (public_argc >= 1) *c++ = MOV_rr(1, 0);

    // r0 = context (literal load)
    // ldr r0, [pc, #imm]
    *c++ = LDR_lit(0, 8); // points 8 bytes past this insn
    // if >=4: make room on stack and store old r3 as first stack arg
    if (public_argc >= 4) {
      *c++ = SUB_sp_imm(8);              // sub sp,sp,#8  (keep 8B align)
      *c++ = STR_imm(12, 13, 0);         // str r12,[sp]
      *c++ = STR_imm(0x0F, 13, 4);       // str r15,[sp,#4] (use r15=pc as zero-ish padding is not ideal; you can write MOV r1,#0; STR r1,[sp,#4] instead)
    }

    // load target into r12 and blx r12
    *c++ = LDR_lit(12, (public_argc >= 4 ? 8 : 4));
    *c++ = 0xE12FFF3C;                   // blx r12

    if (public_argc >= 4) {
      *c++ = ADD_sp_imm(8);              // add sp,sp,#8
    }
    *c++ = 0xE12FFF1E;                   // bx lr

    // Literal pool: context, target
    *c++ = (uint32_t)context;
    *c++ = (uint32_t)target_func;

    __builtin___clear_cache((char*)mem, (char*)mem + SIZE);
    mprotect(mem, SIZE, PROT_READ|PROT_EXEC);
    return mem;
  }

  void trampoline_free(void *trampoline) {
    if (trampoline) munmap(trampoline, SIZE);
  }
