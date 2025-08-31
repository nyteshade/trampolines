// Target: x86-64 SysV (Linux, macOS 10.6–10.8)
// Inserts `context` into rdi, shifts rsi..r9 as needed,
// loads the 6th arg from [rsp+8] and slides any remaining stack args left by one.

#include "trampoline.h"
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#define EMIT8(b)    (*cursor++ = (uint8_t)(b))
#define EMIT32(v)   do { uint32_t _v=(uint32_t)(v); memcpy(cursor,&_v,4); cursor+=4; } while(0)
#define EMIT64(v)   do { uint64_t _v=(uint64_t)(v); memcpy(cursor,&_v,8); cursor+=8; } while(0)

enum { TRAMP_MAX_BYTES = 256 };

static size_t page_size(void) {
  long ps = sysconf(_SC_PAGESIZE);
  return (ps > 0) ? (size_t)ps : 4096u;
}

static void *alloc_rw_page(void) {
  size_t ps = page_size();
  void *p = mmap(NULL, ps, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  return (p == MAP_FAILED) ? NULL : p;
}

void *trampoline_create(void *target_func, void *context, size_t public_argc) {
  uint8_t *buf = (uint8_t *)alloc_rw_page();
  if (!buf) return NULL;
  uint8_t *cursor = buf;

  // Shift GPR args high→low (rsi<-rdi, rdx<-rsi, rcx<-rdx, r8<-rcx, r9<-r8)
  if (public_argc >= 5) { EMIT8(0x4D); EMIT8(0x89); EMIT8(0xC1); } // mov r9,r8
  if (public_argc >= 4) { EMIT8(0x49); EMIT8(0x89); EMIT8(0xC8); } // mov r8,rcx
  if (public_argc >= 3) { EMIT8(0x48); EMIT8(0x89); EMIT8(0xD1); } // mov rcx,rdx
  if (public_argc >= 2) { EMIT8(0x48); EMIT8(0x89); EMIT8(0xF2); } // mov rdx,rsi
  if (public_argc >= 1) { EMIT8(0x48); EMIT8(0x89); EMIT8(0xFE); } // mov rsi,rdi

  // If >=6 public args, bring old stack arg0 into r9 and slide remaining stack args left by 1
  if (public_argc >= 6) {
    // r9 = [rsp+8]  (first stack arg)
    EMIT8(0x4C); EMIT8(0x8B); EMIT8(0x4C); EMIT8(0x24); EMIT8(0x08);

    size_t slide_qwords = public_argc - 6; // how many extra stack args to move
    if (slide_qwords > 0) {
      // r10 = dst (rsp+8), r11 = src (rsp+16), rcx = count
      EMIT8(0x4C); EMIT8(0x8D); EMIT8(0x54); EMIT8(0x24); EMIT8(0x08); // lea r10,[rsp+8]
      EMIT8(0x4C); EMIT8(0x8D); EMIT8(0x5C); EMIT8(0x24); EMIT8(0x10); // lea r11,[rsp+16]
      EMIT8(0x48); EMIT8(0xB9); EMIT64(slide_qwords);                  // mov rcx,imm64

      uint8_t *loop = cursor;
      EMIT8(0x49); EMIT8(0x8B); EMIT8(0x03);             // mov rax,[r11]
      EMIT8(0x49); EMIT8(0x89); EMIT8(0x02);             // mov [r10],rax
      EMIT8(0x49); EMIT8(0x83); EMIT8(0xC3); EMIT8(0x08);// add r11,8
      EMIT8(0x49); EMIT8(0x83); EMIT8(0xC2); EMIT8(0x08);// add r10,8
      EMIT8(0x48); EMIT8(0xFF); EMIT8(0xC9);             // dec rcx
      EMIT8(0x0F); EMIT8(0x85);                          // jnz loop
      int32_t rel = (int32_t)((intptr_t)loop - (intptr_t)(cursor + 4));
      EMIT32(rel);
    }
  }

  // rdi = context
  EMIT8(0x48); EMIT8(0xBF); EMIT64((uint64_t)(uintptr_t)context);
  // rax = target; jmp rax (tail-call)
  EMIT8(0x48); EMIT8(0xB8); EMIT64((uint64_t)(uintptr_t)target_func);
  EMIT8(0xFF); EMIT8(0xE0); // jmp rax

  // RX permissions (round to page)
  if (mprotect(buf, page_size(), PROT_READ | PROT_EXEC) != 0) {
    munmap(buf, page_size());
    return NULL;
  }
  return buf;
}

void trampoline_free(void *tramp) {
  if (!tramp) return;
  munmap((void *)((uintptr_t)tramp & ~((uintptr_t)page_size() - 1)), page_size());
}
