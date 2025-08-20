/*
 * Target: m68k Amiga (AmigaOS 2.x/3.x), GCC 2.95.3, cdecl stack-only ABI.
 * Insert `self` (context) as first argument for any public arity.
 *
 * Sequence:
 *   move.l (sp)+,a0      ; pop caller's return into A0
 *   pea    <context>     ; push context => new arg0
 *   move.l a0,-(sp)      ; push return address back
 *   movea.l #<target>,a1 ; A1 = target
 *   jmp    (a1)          ; tail-jump (target returns to original caller)
 */

#include "trampoline.h"
#include <exec/types.h>
#include <proto/exec.h>

#ifndef CACRF_ClearI
#define CACRF_ClearI 2
#endif

static inline void be_emit16(UBYTE **p, UWORD w) {
  *(*p)++ = (UBYTE)((w >> 8) & 0xFF);
  *(*p)++ = (UBYTE)(w & 0xFF);
}
static inline void be_emit32(UBYTE **p, ULONG x) {
  *(*p)++ = (UBYTE)((x >> 24) & 0xFF);
  *(*p)++ = (UBYTE)((x >> 16) & 0xFF);
  *(*p)++ = (UBYTE)((x >> 8) & 0xFF);
  *(*p)++ = (UBYTE)(x & 0xFF);
}

void *trampoline_create(void *target_func, void *context, size_t public_argc) {
  (void)public_argc; // not needed on stack-only m68k ABI; kept for API uniformity

  // 18 bytes total:
  //  2  move.l (sp)+,a0           0x205F
  //  2+4 pea <context>            0x4879, imm32
  //  2  move.l a0,-(sp)           0x2F08
  //  2+4 movea.l #<target>,a1     0x227C, imm32
  //  2  jmp (a1)                  0x4ED1
  const ULONG kSize = 18;

  UBYTE *code = (UBYTE *)AllocMem(kSize, MEMF_PUBLIC | MEMF_CLEAR);
  if (!code) {
    return NULL;
  }
  UBYTE *c = code;

  be_emit16(&c, 0x205F);                    // move.l (sp)+,a0
  be_emit16(&c, 0x4879); be_emit32(&c, (ULONG)context); // pea <abs.l>
  be_emit16(&c, 0x2F08);                    // move.l a0,-(sp)
  be_emit16(&c, 0x227C); be_emit32(&c, (ULONG)target_func); // movea.l #imm,a1
  be_emit16(&c, 0x4ED1);                    // jmp (a1)

  // Flush I-cache so CPU sees our freshly emitted instructions
  CacheClearE((APTR)code, kSize, CACRF_ClearI);
  return code;
}

void trampoline_free(void *trampoline) {
  if (trampoline) {
    FreeMem(trampoline, 18);
  }
}
