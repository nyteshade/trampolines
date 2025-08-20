#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a trampoline that adapts a public function of arity `public_argc`
 * to a target implementation whose first parameter is an implicit `self`.
 *
 * public signature:  (A1, A2, ..., An)         where n = public_argc
 * target signature:  (void *self, A1, ..., An)
 *
 * On success returns a callable function pointer with the PUBLIC signature.
 */
void *trampoline_create(void *target_func, void *context, size_t public_argc);

/** Free a previously created trampoline. */
void trampoline_free(void *trampoline);

#ifdef __cplusplus
}
#endif

#endif // TRAMPOLINE_H
