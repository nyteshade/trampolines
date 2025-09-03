#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H

#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

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

/* ------------------------------------------------------------------------
 * The following pre-processor defines, setup ways to help define trampoline
 * function prototypes to be nested within structs.
 * ------------------------------------------------------------------------ */

#define TRNullary(functionName) void (*functionName)();

#define TRUnary(returnType, functionName, parameterType) \
  returnType (*functionName)(parameterType);

#define TRDyadic(returnType, functionName, parameterOne, parameterTwo) \
  returnType (*functionName)(parameterOne, parameterTwo);

#define TRProperty(getter, setter, type) \
  type (*getter)(); \
  void (*setter)(type newValue);

#define TRGetter(getter, type) type (*getter)();
#define TRSetter(setter, type) void (*setter)(type newValue);

/* ------------------------------------------------------------------------
 * These pre-processor defines are to actually implement the simpler function
 * variants one might need when working with trampoline enabled code. The
 * following values assume the variable is defined on the public facing struct
 * definition (public variants)
 * ------------------------------------------------------------------------ */

#define TRNullaryFn(return_type, function_name, context_type) \
  return_type function_name(context_type* self)

#define TRUnaryFn(\
  return_type,\
  function_name,\
  context_type,\
  variable_type,\
  variable_name\
) return_type function_name(context_type* self, variable_type variable_name)

#define TRDyadicFn(\
  return_type,\
  function_name,\
  context_type,\
  variable1_type,\
  variable1_name,\
  variable2_type,\
  variable2_name\
) return_type function_name(\
  context_type* self,\
  variable1_type variable1_name,\
  variable2_type variable2_name\
)

#define TRGetterFn(\
  getter,\
  context_type,\
  variable_type,\
  variable\
) variable_type getter(context_type *self) { return self->variable; };

#define TRSetterFn(\
  setter,\
  context_type,\
  variable_type,\
  variable_name\
) void setter(context_type *self, variable_type val) { \
  self->variable_name = val; \
};

#define TRStringSetterFn(\
  setter,\
  context_type,\
  variable_name\
) void setter(context_type *self, const char* val) { \
  if (self->variable_name) { free(self->variable_name); } \
  self->variable_name = calloc(1, sizeof(strlen(val) + 1)); \
  if (self->variable_name) { strcpy(self->variable_name, val); } \
  else { self->variable_name = NULL; } \
};

#define TRPropertyFn(\
  getter,\
  setter,\
  context_type,\
  variable_type,\
  variable_name\
) variable_type getter(context_type *self) { return self->variable; }; \
  void setter(context_type *self, variable_type val) { \
    self->variable_name = val; \
  };

#define TRStringPropertyFn(\
  getter,\
  setter,\
  context_type,\
  variable_name\
) variable_type getter(context_type *self) { return self->variable; }; \
  void setter(context_type *self, const char* val) { \
    if (self->variable_name) { free(self->variable_name); } \
    self->variable_name = calloc(1, sizeof(strlen(val) + 1)); \
    if (self->variable_name) { strcpy(self->variable_name, val); } \
    else { self->variable_name = NULL; } \
  };

/* ------------------------------------------------------------------------
 * These pre-processor defines are to actually implement the simpler function
 * variants one might need when working with trampoline enabled code. The
 * following values assume the variable is defined on a private facing struct
 * definition (private variants)
 * ------------------------------------------------------------------------ */

#define TRPrivateGetterFn(\
  getter,\
  context_type,\
  private_type,\
  variable_type,\
  variable\
) variable_type getter(context_type *self) {\
  private_type* private = (private_type*)self; \
  return private->variable; \
};

#define TRPrivateSetterFn(\
  setter,\
  context_type,\
  private_type,\
  variable_type,\
  variable_name\
) void setter(context_type *self, variable_type val) { \
  private_type* private = (private_type*)self; \
  private->variable_name = val; \
};

#define TRPrivateStringSetterFn(\
  setter,\
  context_type,\
  private_type,\
  variable_name\
) void setter(context_type *self, const char* val) { \
  private_type* private = (private_type*)self; \
  if (private->variable_name) { free(private->variable_name); } \
  private->variable_name = calloc(1, sizeof(strlen(val) + 1)); \
  if (private->variable_name) { strcpy(private->variable_name, val); } \
  else { private->variable_name = NULL; } \
};

#define TRPrivatePropertyFn(\
  getter,\
  setter,\
  context_type,\
  private_type,\
  variable_type,\
  variable_name\
) variable_type getter(context_type *self) {\
    private_type* private = (private_type*)self; \
    return private->variable; \
  }; \
  void setter(context_type *self, variable_type val) { \
    private_type* private = (private_type*)self; \
    private->variable_name = val; \
  };

#define TRPrivateStringPropertyFn(\
  getter,\
  setter,\
  context_type,\
  private_type,\
  variable_name\
) variable_type getter(context_type *self) { \
    private_type* private = (private_type*)self; \
    return private->variable; \
  }; \
  void setter(context_type *self, const char* val) { \
    private_type* private = (private_type*)self; \
    if (private->variable_name) { free(private->variable_name); } \
    private->variable_name = calloc(1, sizeof(strlen(val) + 1)); \
    if (private->variable_name) { strcpy(private->variable_name, val); } \
    else { private->variable_name = NULL; } \
  };

/* ------------------------------------------------------------------------
 * These macros are deprecated and subject to removal once I get around to
 * stripping it from the examples. Expect these to fail soon.
 * ------------------------------------------------------------------------ */

#define TRAMP_PUB2PRIVATE(public_variable, private_type) \
  ((private_type)public_variable)

#define TRAMP_PRIVATE2PUB(private_variable, public_type) \
  ((public_type)private_variable)

#define TRAMP_PROPERTY(name, context_type, variable_type, variable) \
  variable_type get_##name(context_type *self) { \
    return self->variable; \
  } \
  void set_##name(context_type *self, variable_type newValue) { \
    self->variable = newValue; \
  }

#define TRAMP_PROPERTY_(name, context_type, private_variable_type, variable_type, variable) \
  variable_type get_private_##name(context_type *self) { \
    private_variable_type *private = (private_variable_type *)self; \
    return private->variable; \
  } \
  void set_private_##name(context_type *self, variable_type newValue) { \
    private_variable_type *private = (private_variable_type *)self; \
    private->variable = newValue; \
  }

#define TRAMP_STRING_PROPERTY(name, context_type, variable) \
  const char* get_##name(context_type *self) { \
    return self->variable; \
  } \
  void set_##name(context_type *self, const char* newValue) { \
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

#define TRAMP_STRING_PROPERTY_(name, context_type, private_variable_type, variable) \
  const char* get_private_##name(context_type *self) { \
    private_variable_type *private = (private_variable_type *)self; \
    return private->variable; \
  } \
  void set_private_##name(context_type *self, const char* newValue) { \
    private_variable_type *private = (private_variable_type *)self; \
    if (private->variable) { \
      free(private->variable); \
    } \
    private->variable = calloc(1, strlen(newValue) + 1); \
    if (private->variable) { \
      strcpy(private->variable, newValue); \
    } \
    else { \
      private->variable = NULL; \
    } \
  }


#define TRAMP_FREE_PROPERTY(name, context_type, variable_type, variable) \
  variable_type get_##name(context_type *self) { \
    return self->variable; \
  } \
  void set_freeing_##name(context_type *self, variable_type newValue) { \
    if (self->variable) { \
      free(self->variable); \
    } \
    self->variable = newValue; \
  }

#define TRAMP_FREE_PROPERTY_(name, context_type, private_variable_type, variable_type, variable) \
  variable_type get_##name(context_type *self) { \
    private_variable_type *private = (private_variable_type *)self; \
    return private->variable; \
  } \
  void set_freeing_##name(context_type *self, variable_type newValue) { \
    private_variable_type *private = (private_variable_type *)self; \
    if (private->variable) { \
      free(private->variable); \
    } \
    private->variable = newValue; \
  }

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

#define TRAMP_SETTER_(name, context_type, private_variable_type, variable_type, variable) \
  void name(context_type *self, variable_type newValue) { \
    private_variable_type *private = (private_variable_type *)self; \
    private->variable = newValue; \
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

#define TRAMP_STRING_SETTER_(name, context_type, private_variable_type, variable) \
  void name(context_type *self, const char* newValue) { \
    private_variable_type *private = (private_variable_type *)self; \
    if (private->variable) { \
      free(private->variable); \
    } \
    private->variable = calloc(1, strlen(newValue) + 1); \
    if (private->variable) { \
      strcpy(private->variable, newValue); \
    } \
    else { \
      private->variable = NULL; \
    } \
  }

#define TRAMP_FREE_SETTER(name, context_type, variable_type, variable) \
  void name(context_type *self, variable_type newValue) { \
    if (self->variable) { \
      free(self->variable); \
    } \
    self->variable = newValue; \
  }

#ifdef __cplusplus
}
#endif
#endif /* TRAMPOLINE_H */
