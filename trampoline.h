#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H

#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TRAMP_PUB2PRIVATE(public_variable, private_type) \
  ((private_type)public_variable)

#define TRAMP_PRIVATE2PUB(private_variable, public_type) \
  ((public_type)private_variable)

#define TRAMP_GETTER(name, context_type, variable_type, variable) \
  variable_type name(context_type *self) { \
    return self->variable; \
  }

#define TRAMP_GETTER_(name, context_type, private_variable_type, variable_type, variable) \
  variable_type name(context_type *self) { \
    private_variable_type *private = (private_variable_type *)self; \
    return private->variable; \
  }

#define TRAMP_SETTER(name, context_type, variable_type, variable) \
  void name(context_type *self, variable_type newValue) { \
    self->variable = newValue; \
  }
  
#define TRAMP_STRING_SETTER(name, context_type, variable) \
  void name(context_type *self, const char* newValue) { \
    if (self->variable) { \
      free(self->variable); \
    } \
    self->variable = calloc(1, strlen(newValue) + 1); \
    if (self->variable) { \
      strcpy(self->variable, newValue); \
    } \
    else { \
      self->variable = NULL; \
    } \
  }
  
#define TRAMP_FREE_SETTER(name, context_type, variable_type, variable) \
  void name(context_type *self, variable_type newValue) { \
    if (self->variable) { \
      free(self->variable); \
    } \
    self->variable = newValue; \
  }

/* ------------------------------------------------------------------------ */
/* A structure containing all the allocations made during the process of    */
/* initializing a structure with trampoline functions within. This isn't    */
/* required, but makes the code easier to work with if more than one        */
/* succeeded while only some failed.                                        */
/* ------------------------------------------------------------------------ */

typedef struct trampoline_allocations {
  void* pointers[255];
  unsigned char failures;
  unsigned int next;
} trampoline_allocations;

/* ------------------------------------------------------------------------ */
/* This function takes a `trampoline_allocations` struct and checks to see  */
/* if any failures occurred. If so, all non-null pointers in the struct are */
/* deallocated using trampoline_free() before returning 0 (false). If all   */
/* went well, returning 1 (true).                                           */
/* ------------------------------------------------------------------------ */

unsigned char trampolines_validate(
  trampoline_allocations *allocations
);

/* ------------------------------------------------------------------------ */
/* Determine if we are going to need to declare the number of public args   */
/* ------------------------------------------------------------------------ */

#if defined(AMIGA) || defined(__amigaos__)
#define NO_PUBLIC_ARGC
#else
#define HAS_PUBLIC_ARGC
#endif

/* ------------------------------------------------------------------------ */
/* Declare prototypes for creating and tracking trampoline functions when   */
/* do not have to declare the total number of public arguments              */
/* ------------------------------------------------------------------------ */

#ifdef NO_PUBLIC_ARGC
  void *trampoline_create(void *target_func, void *context);

  /* ---------------------------------------------------------------------- */
  /* The create_and_track function utilizes the non-tracking processor and  */
  /* OS specific variant of trampoline_create() and wraps tracking logic in */
  /* the non-processor specific trampoline_helpers.c file.                  */
  /* ---------------------------------------------------------------------- */    
  
  void *trampoline_create_and_track(
    void *target_func, 
    void *context, 
    trampoline_allocations *allocations
  );

/* ------------------------------------------------------------------------ */
/* Declare prototypes for creating and tracking trampoline functions when   */
/* DO have to declare the total number of public arguments                  */
/* ------------------------------------------------------------------------ */

#else
  void *trampoline_create(void *target_func, void *context, size_t public_argc);
  
  /* ---------------------------------------------------------------------- */
  /* The create_and_track function utilizes the non-tracking processor and  */
  /* OS specific variant of trampoline_create() and wraps tracking logic in */
  /* the non-processor specific trampoline_helpers.c file.                  */
  /* ---------------------------------------------------------------------- */    

  void *trampoline_create_and_track(
    void *target_func, 
    void *context,
    size_t public_args, 
    trampoline_allocations *allocations
  );
#endif

/* ------------------------------------------------------------------------ */
/* Freeing a trampoline is still processor specific, this is defined in the */
/* processor specific implementation file.                                  */
/* ------------------------------------------------------------------------ */

void trampoline_free(void *trampoline);

#ifdef __cplusplus
}
#endif
#endif /* TRAMPOLINE_H */
