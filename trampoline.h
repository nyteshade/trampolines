#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Amiga-family selection: classic m68k AmigaOS, OS4, AROS, MorphOS. */
#if defined(AMIGA) || defined(__amigaos__) || defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__)
/* m68k/OS4/AROS/MorphOS use the 2-arg creator (all args on stack). */
void *trampoline_create(void *target_func, void *context);
#else
/* Other platforms use the 3-arg creator (we may need public_argc). */
void *trampoline_create(void *target_func, void *context, size_t public_argc);
#endif

void trampoline_free(void *trampoline);

#ifdef __cplusplus
}
#endif
#endif /* TRAMPOLINE_H */
