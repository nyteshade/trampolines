/*
 * Amiga m68k (AmigaOS 2.x/3.x), GCC 2.95.3, cdecl stack-only ABI.
 * Robust variant: saves the caller's return in a scratch slot placed
 * *after* the instruction stream to avoid overwriting code (including RTS).
 *
 * Code stream is 32 bytes. We allocate 36 and put scratch at code+32.
 *
 * Emitted instructions (32 bytes total):
 *   205F              movea.l (sp)+,a0                 ; A0 = caller_ret
 *   23C8 <SS>         move.l  a0,(abs).l scratch       ; save caller_ret
 *   4879 <CC>         pea     (abs).l context          ; push context
 *   227C <TT>         movea.l #<target>,a1             ; A1 = target
 *   4E91              jsr     (a1)                     ; call target(self,...)
 *   588F              addq.l  #4,sp                    ; drop context
 *   2F39 <SS>         move.l  (abs).l scratch,-(sp)    ; restore caller_ret
 *   4E75              rts                              ; return to caller
 *
 * [SS] = 32-bit absolute address of scratch (code+32)
 * [CC] = 32-bit absolute address of context
 * [TT] = 32-bit absolute address of target
 */

#define TRAMPOLINE_M68K_VER 0x00010002 /* 1.2 */

#include "trampoline.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#ifdef __has_include
# if __has_include(<exec/execbase.h>)
#  include <exec/execbase.h>
# endif
#endif

#ifndef MEMF_PUBLIC
#define MEMF_PUBLIC 0
#endif
#ifndef CACRF_ClearI
#define CACRF_ClearI (1L << 1)
#endif

/* 32 bytes of code + 4 bytes of data (scratch) */
#define M68K_CODE_SIZE      32
#define M68K_TOTAL_SIZE     36

typedef struct be_ctx_s {
  UBYTE *p;
  UBYTE *base;
  ULONG cap;
} be_ctx;

static void be_emit8(be_ctx *c, UBYTE v)
{
  if ((ULONG)(c->p - c->base) < c->cap) { *(c->p) = v; c->p++; }
}
static void be_emit16(be_ctx *c, UWORD v)
{
  be_emit8(c, (UBYTE)(v >> 8));
  be_emit8(c, (UBYTE)(v));
}
static void be_emit32(be_ctx *c, ULONG v)
{
  be_emit8(c, (UBYTE)(v >> 24));
  be_emit8(c, (UBYTE)(v >> 16));
  be_emit8(c, (UBYTE)(v >> 8));
  be_emit8(c, (UBYTE)(v));
}

void *trampoline_create(void *target_func, void *context)
{
  UBYTE *code;
  be_ctx c;
  ULONG scratch;

  code = (UBYTE *)AllocMem(M68K_TOTAL_SIZE, MEMF_PUBLIC);
  if (!code) return 0;

  c.p = code;
  c.base = code;
  c.cap = M68K_TOTAL_SIZE;

  scratch = (ULONG)(code + M68K_CODE_SIZE); /* safe: after the 32-byte code */

  /* movea.l (sp)+,a0 */
  be_emit16(&c, 0x205F);

  /* move.l a0,(abs).l scratch  ; 23C8 + abs.l */
  be_emit16(&c, 0x23C8);
  be_emit32(&c, scratch);

  /* pea <abs.l context>        ; 4879 + abs.l */
  be_emit16(&c, 0x4879);
  be_emit32(&c, (ULONG)context);

  /* movea.l #<target>,a1       ; 227C + abs.l */
  be_emit16(&c, 0x227C);
  be_emit32(&c, (ULONG)target_func);

  /* jsr (a1)                    ; 4E91 */
  be_emit16(&c, 0x4E91);

  /* addq.l #4,sp                ; 588F */
  be_emit16(&c, 0x588F);

  /* move.l (abs).l scratch,-(sp); 2F39 + abs.l */
  be_emit16(&c, 0x2F39);
  be_emit32(&c, scratch);

  /* rts                         ; 4E75 */
  be_emit16(&c, 0x4E75);

  /* Optional: initialize scratch (now safely *outside* the code) */
  code[M68K_CODE_SIZE + 0] = 0;
  code[M68K_CODE_SIZE + 1] = 0;
  code[M68K_CODE_SIZE + 2] = 0;
  code[M68K_CODE_SIZE + 3] = 0;

  CacheClearE((APTR)code, (ULONG)M68K_TOTAL_SIZE, CACRF_ClearI);
  return (void *)code;
}

void trampoline_free(void *trampoline)
{
  if (trampoline) {
    FreeMem((APTR)trampoline, M68K_TOTAL_SIZE);
  }
}
