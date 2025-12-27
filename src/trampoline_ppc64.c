/* trampoline_ppc64.c — Mac OS X 10.5 (PPC64 userland on G5)
 * C89; generates a tail-call stub + inline literals (context, target).
 *
 * ABI refs (Darwin PPC64):
 * - GPR args r3..r10, parameter area slots SP+48..+104; overflow begins at SP+112.
 *   Apple "Mac OS X ABI Function Call Guide".                                  
 */

#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "trampoline.h"

#ifndef PROT_EXEC
# define PROT_EXEC 0x04
#endif
#ifndef MAP_ANON
# define MAP_ANON MAP_ANONYMOUS
#endif

extern void __clear_cache(char *b, char *e);

typedef uint32_t u32;
typedef uint64_t u64;
#define PPC_EMIT(w)  do { *p++ = (u32)(w); } while (0)

/* --- DS-form helpers for LD/STD (offset must be 4-byte multiple) --- */
static u32 ppc64_ld(int rt, int ra, unsigned ds /* offset/4 */) {
  /* op=58, RT, RA, DS(14), XO=00 */
  return (58u<<26) | ((rt&31)<<21) | ((ra&31)<<16) | ((ds&0x3FFF)<<2);
}
static u32 ppc64_std(int rs, int ra, unsigned ds /* offset/4 */) {
  /* op=62, RS, RA, DS(14), XO=00 */
  return (62u<<26) | ((rs&31)<<21) | ((ra&31)<<16) | ((ds&0x3FFF)<<2);
}

/* addi RT,RA,sim16 (works on 64-bit GPRs too) */
static u32 ppc_addi(int rt, int ra, int16_t simm) {
  return (14u<<26) | ((rt&31)<<21) | ((ra&31)<<16) | ((uint16_t)simm);
}
/* mfspr (LR=8) / mflr RT  — XO=339 */
static u32 ppc_mflr(int rt) {
  unsigned spr = 8; /* LR */
  unsigned sprfld = ((spr & 0x1f) << 16) | ((spr & 0x3e0) << 6);
  return (31u<<26) | ((rt&31)<<21) | sprfld | (339u<<1);
}
/* mtspr (LR=8) / mtlr RS  — XO=467 */
static u32 ppc_mtlr(int rs) {
  unsigned spr = 8; /* LR */
  unsigned sprfld = ((spr & 0x1f) << 16) | ((spr & 0x3e0) << 6);
  return (31u<<26) | ((rs&31)<<21) | sprfld | (467u<<1);
}
/* mtspr (CTR=9) / mtctr RS */
static u32 ppc_mtctr(int rs) {
  unsigned spr = 9; /* CTR */
  unsigned sprfld = ((spr & 0x1f) << 16) | ((spr & 0x3e0) << 6);
  return (31u<<26) | ((rs&31)<<21) | sprfld | (467u<<1);
}
/* bctr */
static u32 ppc_bctr(void) { return 0x4E800420u; }
/* bl to PC+imm (imm is byte offset / 4 encoded by assembler; here we pass raw LI).
   Encoding: op=18, LI(24) << 2, AA=0, LK=1 */
static u32 ppc_bl_rel24(int32_t byte_disp) {
  /* byte_disp must be divisible by 4; range ±32MB is plenty for our tiny stub */
  uint32_t LI = ((uint32_t)byte_disp >> 2) & 0x00FFFFFFu;
  return (18u<<26) | LI | 1u; /* LK=1 */
}

/* ----------------------------- public API --------------------------------- */

void *trampoline_create(void *target_func, void *context, size_t public_argc)
{
  /* We place two 8-byte literals after the code:
     [context (8)] [target (8)]
     and fetch them PC-rel using a local bl/mflr sequence.
  */
  u32 *code;
  u32 *p;
  long pagesz;
  size_t code_words, moves, need_spill;
  size_t total_bytes;
  u64 *lit;

  if (public_argc > 64) public_argc = 64;
  need_spill = (public_argc >= 8) ? 1u : 0u;
  moves = (public_argc >= 1) ? (public_argc < 8 ? public_argc : 7) : 0;

  /* Code plan (words):
     mflr r0                     1
     bl   +8                     1
   L: mflr r12                   1
      addi r12,r12, +N           1 (N = bytes from here to literals; fits 16-bit)
      ld   r3,  0(r12)           1 (context)
      ld   r12, 8(r12)           1 (target)
      mtlr r0                    1
      optional spill (std r10,112(r1))         1 if needed
      moves r10<-r9 .. r4<-r3                up to 7
      addi r3,r3,0  (r3 already = context)   0 (we already loaded r3)
      mtctr r12                  1
      bctr                       1
     literals: context (8), target (8)
  */
  code_words = 1+1+1+1+1+1+1 + need_spill + moves + 1 + 1;
  pagesz = sysconf(_SC_PAGESIZE);
  if (pagesz <= 0) pagesz = 4096;

  /* Total = code + 16 bytes of literals; round to page. */
  total_bytes = (code_words*4 + 16u + pagesz-1) & ~(pagesz-1);

  code = (u32*)mmap(0, total_bytes,
                    PROT_READ|PROT_WRITE|PROT_EXEC,
                    MAP_PRIVATE|MAP_ANON, -1, 0);
  if (!code || code == (void*)-1) return 0;

  p = code;

  /* 1) Save/borrow LR to fetch PC, then restore LR before tail-call. */
  PPC_EMIT(ppc_mflr(0));                 /* mflr r0 */
  PPC_EMIT(ppc_bl_rel24(8));             /* bl  +8  -> next insn + 8 bytes */

  /* 2) Compute address of literals and load context/target. */
  /* label target of bl: */
  /* (PC here) */
  PPC_EMIT(ppc_mflr(12));                /* mflr r12 (PC) */
  /* Offset from *here* to literals: we'll place literals immediately after code.
     We don't know final offset until we lay them; so compute it now: */
  {
    /* bytes from after this addi to start of literals: */
    size_t bytes_to_here = (size_t)((char*)p - (char*)code);
    size_t code_bytes_total = (code_words*4);  /* without literals */
    int16_t addi_off = (int16_t)((int32_t)(code_bytes_total - bytes_to_here));
    PPC_EMIT(ppc_addi(12, 12, addi_off));      /* addi r12,r12, +off */
  }
  PPC_EMIT(ppc64_ld(3, 12, 0/4));        /* ld r3,  0(r12)  ; context */
  PPC_EMIT(ppc64_ld(12,12, 8/4));        /* ld r12, 8(r12)  ; target  */
  PPC_EMIT(ppc_mtlr(0));                 /* mtlr r0         ; restore original LR */

  /* 3) If 8+ args, spill old r10 to first overflow slot (SP+112). */
  if (need_spill) {
    PPC_EMIT(ppc64_std(10, 1, 112/4));   /* std r10,112(r1) */
  }

  /* 4) Shift reg args upward (same pattern as PPC32). */
  if (moves >= 7) PPC_EMIT(ppc_addi(10,  9, 0));
  if (moves >= 6) PPC_EMIT(ppc_addi( 9,  8, 0));
  if (moves >= 5) PPC_EMIT(ppc_addi( 8,  7, 0));
  if (moves >= 4) PPC_EMIT(ppc_addi( 7,  6, 0));
  if (moves >= 3) PPC_EMIT(ppc_addi( 6,  5, 0));
  if (moves >= 2) PPC_EMIT(ppc_addi( 5,  4, 0));
  if (moves >= 1) PPC_EMIT(ppc_addi( 4,  3, 0));  /* r3 holds context already */

  /* 5) Tail-call. */
  PPC_EMIT(ppc_mtctr(12));
  PPC_EMIT(ppc_bctr());

  /* 6) Literals: [context][target] immediately after code. */
  lit = (u64*)( (char*)code + code_words*4 );
  lit[0] = (u64)(uintptr_t)context;
  lit[1] = (u64)(uintptr_t)target_func;

  __clear_cache((char*)code, (char*)code + total_bytes);
  return (void*)code;
}

void trampoline_free(void *trampoline)
{
  if (trampoline) {
    long pagesz = sysconf(_SC_PAGESIZE);
    if (pagesz <= 0) pagesz = 4096;
    munmap(trampoline, (size_t)pagesz);
  }
}
