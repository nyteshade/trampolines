#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H

// Creates a trampoline that calls 'target_func' with 'context' as the first argument.
void *trampoline_create(void *target_func, void *context);

// Frees the memory allocated for a trampoline.
void trampoline_free(void *trampoline);

#endif // TRAMPOLINE_H
