// Target: x86-64 SysV (Linux, macOS)
// Inserts self into rdi, shifts GPRs right by 1, slides stack left if needed.
// public_argc = number of public args (A1..An)

#include "trampoline.h"
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <pthread.h>
#ifndef MAP_JIT
#define MAP_JIT 0x0800
#endif
#endif

#define EMIT8(b)    (*cursor++ = (uint8_t)(b))
#define EMIT32(v)   do { uint32_t _v=(uint32_t)(v); memcpy(cursor,&_v,4); cursor+=4; } while(0)
#define EMIT64(v)   do { uint64_t _v=(uint64_t)(v); memcpy(cursor,&_v,8); cursor+=8; } while(0)

enum { TRAMP_MAX_BYTES = 256 };

static void *alloc_rw(size_t sz) {
  size_t ps = (size_t)sysconf(_SC_PAGESIZE);
  size_t need = (sz + ps - 1) & ~(ps - 1);
#if defined(__APPLE__)
  void *p = mmap(NULL, need, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON|MAP_JIT, -1, 0);
#else
  void *p = mmap(NULL, need, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#endif
  return p == MAP_FAILED ? NULL : p;
}

static int to_rx(void *p, size_t sz) {
  size_t ps = (size_t)sysconf(_SC_PAGESIZE);
  size_t need = (sz + ps - 1) & ~(ps - 1);
#if defined(__APPLE__)
  pthread_jit_write_protect_np(1);
  int rc = mprotect(p, need, PROT_READ|PROT_EXEC);
  pthread_jit_write_protect_np(0);
  return rc;
#else
  return mprotect(p, need, PROT_READ|PROT_EXEC);
#endif
}

void *trampoline_create(void *target_func, void *context, size_t public_argc) {
  uint8_t *buf = (uint8_t *)alloc_rw(TRAMP_MAX_BYTES);
  if (!buf) return NULL;
  uint8_t *cursor = buf;

  // Shift registers (do high→low to avoid clobber)
  if (public_argc >= 5) { EMIT8(0x4D); EMIT8(0x89); EMIT8(0xC1); } // mov r9,r8
  if (public_argc >= 4) { EMIT8(0x49); EMIT8(0x89); EMIT8(0xC8); } // mov r8,rcx
  if (public_argc >= 3) { EMIT8(0x48); EMIT8(0x89); EMIT8(0xD1); } // mov rcx,rdx
  if (public_argc >= 2) { EMIT8(0x48); EMIT8(0x89); EMIT8(0xF2); } // mov rdx,rsi
  if (public_argc >= 1) { EMIT8(0x48); EMIT8(0x89); EMIT8(0xFE); } // mov rsi,rdi

  // If >=6 public args, arg6 was at [rsp+8]. Move it to r9 and slide the stack left by 1
  if (public_argc >= 6) {
    // mov r9, [rsp+8]
    EMIT8(0x4C); EMIT8(0x8B); EMIT8(0x4C); EMIT8(0x24); EMIT8(0x08);

    size_t slide_qwords = public_argc - 6;
    if (slide_qwords > 0) {
      // lea r10, [rsp+8] ; lea r11, [rsp+16] ; mov rcx, slide_qwords
      EMIT8(0x4C); EMIT8(0x8D); EMIT8(0x54); EMIT8(0x24); EMIT8(0x08);
      EMIT8(0x4C); EMIT8(0x8D); EMIT8(0x5C); EMIT8(0x24); EMIT8(0x10);
      EMIT8(0x48); EMIT8(0xB9); EMIT64(slide_qwords);

      uint8_t *loop = cursor;
      EMIT8(0x49); EMIT8(0x8B); EMIT8(0x03);       // mov rax,[r11]
      EMIT8(0x49); EMIT8(0x89); EMIT8(0x02);       // mov [r10],rax
      EMIT8(0x49); EMIT8(0x83); EMIT8(0xC3); EMIT8(0x08); // add r11,8
      EMIT8(0x49); EMIT8(0x83); EMIT8(0xC2); EMIT8(0x08); // add r10,8
      EMIT8(0x48); EMIT8(0xFF); EMIT8(0xC9);       // dec rcx
      EMIT8(0x0F); EMIT8(0x85);                    // jnz loop
      int32_t rel = (int32_t)((intptr_t)loop - (intptr_t)(cursor + 4));
      EMIT32(rel);
    }
  }

  // rdi = context
  EMIT8(0x48); EMIT8(0xBF); EMIT64((uint64_t)context);
  // rax = target; jmp rax
  EMIT8(0x48); EMIT8(0xB8); EMIT64((uint64_t)target_func);
  EMIT8(0xFF); EMIT8(0xE0);

  size_t used = (size_t)(cursor - buf);
  if (to_rx(buf, used) != 0) { munmap(buf, used); return NULL; }
  return buf;
}

void trampoline_free(void *tramp) {
  if (!tramp) return;
  size_t ps = (size_t)sysconf(_SC_PAGESIZE);
  munmap((void *)((uintptr_t)tramp & ~(ps - 1)), ps);
}
