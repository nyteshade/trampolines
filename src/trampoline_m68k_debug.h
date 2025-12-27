#ifndef TRAMPOLINE_M68K_DEBUG_H
#define TRAMPOLINE_M68K_DEBUG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Amiga-family selection: classic m68k AmigaOS, OS4, AROS, MorphOS. */
#if defined(AMIGA) || defined(__amigaos__) || defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__)
void dump_tramp(const char *label, const void *p, unsigned len);
#endif

#ifdef __cplusplus
}

#endif
#endif /* TRAMPOLINE_M68K_DEBUG_H */
