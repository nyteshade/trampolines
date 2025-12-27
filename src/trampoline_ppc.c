/* trampoline_ppc.c — Mac OS X 10.3/10.4/10.5 (PPC32)
 * C89; generates a small tail-call stub in executable memory.
 *
 * ABI refs (Darwin PPC32):
 * - GPR args r3..r10, overflow begins at SP+56; param save area SP+24..+52. 
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

/* GCC builtin (available on GCC 3.x/4.x). */
extern void __clear_cache(char *b, char *e);

/* ---------- PPC32 instruction encoders (big-endian 32-bit words) ---------- */

typedef uint32_t u32;

#define PPC_EMIT(w)  do { *p++ = (u32)(w); } while (0)

/* addis RT,RA,imm16  (lis when RA=0) */
static u32 ppc_addis(int rt, int ra, uint16_t imm) {
  return (15u<<26) | ((rt&31)<<21) | ((ra&31)<<16) | (imm&0xFFFF);
}
/* ori RA,RS,uimm16 */
static u32 ppc_ori(int ra, int rs, uint16_t imm) {
  return (24u<<26) | ((rs&31)<<21) | ((ra&31)<<16) | (imm&0xFFFF);
}
/* addi RT,RA,sim16 — we use addi rd,rs,0 as a register move ("mr"). */
static u32 ppc_addi(int rt, int ra, int16_t simm) {
  return (14u<<26) | ((rt&31)<<21) | ((ra&31)<<16) | ((uint16_t)simm);
}
/* stw RS,D(RA) */
static u32 ppc_stw(int rs, int ra, int16_t d) {
  return (36u<<26) | ((rs&31)<<21) | ((ra&31)<<16) | ((uint16_t)d);
}
/* mtspr (CTR=9) / mtctr RS  — mtspr uses split SPR field */
static u32 ppc_mtctr(int rs) {
  unsigned spr = 9; /* CTR */
  unsigned sprfld = ((spr & 0x1f) << 16) | ((spr & 0x3e0) << 6);
  return (31u<<26) | ((rs&31)<<21) | sprfld | (467u<<1); /* Rc=0 */
}
/* bctr (branch to CTR, no link) — fixed encoding */
static u32 ppc_bctr(void) { return 0x4E800420u; }

static void split32(uint32_t val, uint16_t *hi, uint16_t *lo) {
  *hi = (uint16_t)(val >> 16);
  *lo = (uint16_t)(val & 0xFFFFu);
}

/* ----------------------------- public API --------------------------------- */

void *trampoline_create(void *target_func, void *context, size_t public_argc)
{
  u32 *code;
  u32 *p;
  size_t words, moves, need_spill;

  uint16_t ctx_hi, ctx_lo, tgt_hi, tgt_lo;
  long pagesz;
  size_t bytes;

  /* Clamp to the eight integer/pointer arg regs (r3..r10). */
  if (public_argc > 32) public_argc = 32; /* sanity */
  need_spill = (public_argc >= 8) ? 1u : 0u;
  moves = (public_argc >= 1) ? (public_argc < 8 ? public_argc : 7) : 0;

  /* Instruction count:
     - load context: 2
     - load target:  2
     - optional spill r10 -> 56(sp): 1
     - register moves r3..r? : moves
     - move r3 <- r11: 1
     - mtctr + bctr: 2
  */
  words = 2 + 2 + need_spill + moves + 1 + 2;
  bytes = words * 4;

  pagesz = sysconf(_SC_PAGESIZE);
  if (pagesz <= 0) pagesz = 4096;

  /* RWX mapping (PPC Tiger/Leopard allow this). */
  code = (u32*)mmap(0, (bytes + pagesz-1) & ~(pagesz-1),
                    PROT_READ|PROT_WRITE|PROT_EXEC,
                    MAP_PRIVATE|MAP_ANON, -1, 0);
  if (!code || code == (void*)-1) return 0;

  p = code;

  /* Load r11 = context, r12 = target (32-bit addresses on PPC32). */
  split32((uint32_t)(uintptr_t)context, &ctx_hi, &ctx_lo);
  split32((uint32_t)(uintptr_t)target_func, &tgt_hi, &tgt_lo);
  PPC_EMIT(ppc_addis(11, 0, ctx_hi));   /* lis   r11, hi16(context) */
  PPC_EMIT(ppc_ori  (11,11, ctx_lo));   /* ori   r11,r11,lo16(context) */
  PPC_EMIT(ppc_addis(12, 0, tgt_hi));   /* lis   r12, hi16(target)   */
  PPC_EMIT(ppc_ori  (12,12, tgt_lo));   /* ori   r12,r12,lo16(target)*/

  /* If 8+ public args, the old r10 becomes the 9th => store to SP+56. */
  if (need_spill) {
    PPC_EMIT(ppc_stw(10, 1, 56));       /* stw r10,56(r1) */
  }

  /* Shift r3..r? upward by one slot (mr via addi rd,rs,0):
     r10<-r9, r9<-r8, ..., r4<-r3  (at most 7 moves).
  */
  if (moves >= 7) PPC_EMIT(ppc_addi(10,  9, 0));
  if (moves >= 6) PPC_EMIT(ppc_addi( 9,  8, 0));
  if (moves >= 5) PPC_EMIT(ppc_addi( 8,  7, 0));
  if (moves >= 4) PPC_EMIT(ppc_addi( 7,  6, 0));
  if (moves >= 3) PPC_EMIT(ppc_addi( 6,  5, 0));
  if (moves >= 2) PPC_EMIT(ppc_addi( 5,  4, 0));
  if (moves >= 1) PPC_EMIT(ppc_addi( 4,  3, 0));

  /* r3 = context */
  PPC_EMIT(ppc_addi(3, 11, 0));

  /* Jump to target without touching LR (tail-call). */
  PPC_EMIT(ppc_mtctr(12));
  PPC_EMIT(ppc_bctr());

  /* Make it executable for the I-cache. */
  __clear_cache((char*)code, (char*)code + bytes);
  return (void*)code;
}

void trampoline_free(void *trampoline)
{
  if (trampoline) {
    long pagesz = sysconf(_SC_PAGESIZE);
    if (pagesz <= 0) pagesz = 4096;
    /* We don’t know exact size here; free at least one page (all our stubs are < 1 page). */
    munmap(trampoline, (size_t)pagesz);
  }
}
