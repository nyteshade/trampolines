/*
 * Target: Motorola 68000 (m68k)
 * Platform: Commodore Amiga (AmigaOS 2.x/3.x)
 * Compiler: GCC 2.95.3
 * ABI: Arguments passed on the stack (right to left).
 */

#include "trampoline.h"
#include <proto/exec.h> /* AmigaOS specific includes */
#include <dos/dos.h>  /* For error codes */

/*
 * We need access to the Exec library base to call its functions.
 * This is a global pointer that is set up by the C startup code.
 */
extern struct ExecBase *SysBase;

/*
 * The size of our dynamically generated function in bytes.
 * This is calculated based on the 68k instructions we will write.
 */
#define TRAMPOLINE_SIZE 20

void *trampoline_create(void *target_func, void *context) {
  /*
   * On AmigaOS, we allocate memory using AllocMem from exec.library.
   * MEMF_PUBLIC: Memory that is visible to all tasks.
   * MEMF_CLEAR:  Initialize the memory to all zeros.
   * On classic Amigas, there is no hardware distinction between executable
   * and writable memory, so we don't need special permissions like on modern systems.
   */
  unsigned short *code = AllocMem(TRAMPOLINE_SIZE, MEMF_PUBLIC | MEMF_CLEAR);
  if (!code) {
    return NULL;
  }

  /*
   * Now we write the 68000 machine code directly into the allocated memory.
   * The 68k is a "big-endian" processor, and instructions are 16-bits (a short).
   * We use specific hexadecimal codes that correspond to assembly instructions.
   */

  /* Instruction 1: move.l #<context_addr>, a0 */
  /* Loads the 32-bit address of our 'context' into address register A0. */
  code[0] = 0x207c; /* Opcode for movea.l immediate */
  *(void **)(code + 1) = context; /* Write the 32-bit context address */

  /* Instruction 2: move.l #<target_func_addr>, a1 */
  /* Loads the 32-bit address of our target function into address register A1. */
  code[3] = 0x227c; /* Opcode for movea.l immediate (to A1) */
  *(void **)(code + 4) = target_func; /* Write the 32-bit function address */

  /* Instruction 3: move.l a0, -(sp) */
  /* Pushes the context pointer (from A0) onto the stack. This becomes the first argument. */
  code[6] = 0x2f08; /* Opcode for move.l a0, -(sp) */

  /* Instruction 4: jsr (a1) */
  /* Jumps to the subroutine located at the address in register A1. */
  code[7] = 0x4e91; /* Opcode for jsr (a1) */

  /* Instruction 5: addq.l #4, sp */
  /* Cleans up the stack by removing the argument we pushed. */
  code[8] = 0x588f; /* Opcode for addq.l #4, sp */

  /* Instruction 6: rts */
  /* Return from Subroutine. This returns control to the original caller. */
  code[9] = 0x4e75; /* Opcode for rts */


  /*
   * The CPU might have an old, empty copy of this memory in its instruction cache.
   * We must flush the cache to ensure it reads our new instructions.
   * CacheClearE is the AmigaOS function to do this.
   */
  CacheClearE(code, TRAMPOLINE_SIZE, CACRF_ClearI);

  return code;
}

void trampoline_free(void *trampoline) {
  if (trampoline) {
    /*
     * Free the memory using the corresponding AmigaOS function.
     */
    FreeMem(trampoline, TRAMPOLINE_SIZE);
  }
}
