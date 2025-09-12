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

typedef struct TTAllocNode {
  struct TTAllocNode* next;
  void* trampoline;
} TTAllocNode;

typedef struct TTTracker {
  struct TTTracker* next;
  TTAllocNode* first;
  void* context;

  unsigned int failures;
  unsigned int count;
  unsigned long id;
} TTTracker;

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
  void *trampoline_create(void* target_func, void* context);

  /**
   * Creates a trampoline and registers it in the global list of context to
   * trampolines list.
   *
   * These are stored in a TTTracker instance and each subsequent trampoline
   * created and stored in this list can help determine whether or not the
   * creation of each trampoline as a whole was successful and also provides
   * a method by which to free the memory of each trampoline for a given
   * struct easily.
   *
   * @param target_func the original function pointer that will be given context
   * on each execution.
   *
   * @param context the context to supply as the `self` parameter for each
   * call to the aforementioned target_func (i.e. the this object)
   *
   * @param tracker a pointer to a pointer to a TTTracker to help optimize the
   * adding of a given trampoline to the tracked memory structures. If this is
   * NULL, additional checks will be performed, and searches of known structures
   * executed, before a new TTTracker instance is created to manage the
   * trampolines for a given context.
   *
   * @return a pointer to the trampoline that was created or NULL if an
   * error occurred.
   */
  void *trampoline_monitor(
    void* target_func,
    void* context,
    TTTracker** tracker
  );
#else
  void *trampoline_create(void *target_func, void *context, size_t public_argc);

  /**
   * Creates a trampoline and registers it in the global list of context to
   * trampolines list.
   *
   * These are stored in a TTTracker instance and each subsequent trampoline
   * created and stored in this list can help determine whether or not the
   * creation of each trampoline as a whole was successful and also provides
   * a method by which to free the memory of each trampoline for a given
   * struct easily.
   *
   * @param target_func the original function pointer that will be given context
   * on each execution.
   *
   * @param context the context to supply as the `self` parameter for each
   * call to the aforementioned target_func (i.e. the this object)
   *
   * @param public_argc on some CPUs or operating systems, calculation of stack
   * pointer movement needs to be explicit, the public_argc is the number of
   * parametes the trampoline receives aside from the implicit self/context
   * provided as the first parameter.
   *
   * @param tracker a pointer to a pointer to a TTTracker to help optimize the
   * adding of a given trampoline to the tracked memory structures. If this is
   * NULL, additional checks will be performed, and searches of known structures
   * executed, before a new TTTracker instance is created to manage the
   * trampolines for a given context.
   *
   * @return a pointer to the trampoline that was created or NULL if an
   * error occurred.
   */
  void *trampoline_monitor(
    void* target_func,
    void* context,
    size_t public_argc,
    TTTracker** tracker
  );
#endif

/* ------------------------------------------------------------------------ */
/* Freeing a trampoline is still processor specific, this is defined in the */
/* processor specific implementation file.                                  */
/* ------------------------------------------------------------------------ */

void trampoline_free(void *trampoline);

/**
 * Destroys a TTTracker and all its associated trampolines.
 *
 * This function frees all trampolines tracked by it, deallocates all
 * TTAllocNode structures, removes the TTTracker from the global list,
 * and finally frees the TTTracker itself.
 *
 * @param context The context pointer identifying which TTTracker to destroy.
 * If NULL or if no matching tracker is found, the function returns 0.
 *
 * @return The number of trampolines that were freed, or 0 if no tracker was found
 * or if the tracker was the global static tracker.
 *
 * @note This function calls trampoline_free() for each trampoline before cleanup.
 * The global __trampolines tracker cannot be freed and will return 0.
 *
 * @warning After calling this function, any pointers to the TTTracker or its
 * TTAllocNodes become invalid and must not be used.
 */
unsigned int trampoline_tracker_free(TTTracker* tracker);

/**
 * Destroys a TTTracker and all its associated trampolines for the given context.
 *
 * This function locates the TTTracker associated with the specified context,
 * frees all trampolines tracked by it, deallocates all TTAllocNode structures,
 * removes the TTTracker from the global list, and finally frees the TTTracker itself.
 *
 * @param context The context pointer identifying which TTTracker to destroy.
 *                If NULL or if no matching tracker is found, the function returns 0.
 *
 * @return The number of trampolines that were freed, or 0 if no tracker was found
 *         or if the tracker was the global static tracker.
 *
 * @note This function calls trampoline_free() for each trampoline before cleanup.
 *       The global __trampolines tracker cannot be freed and will return 0.
 *
 * @warning After calling this function, any pointers to the TTTracker or its
 *          TTAllocNodes become invalid and must not be used.
 */
unsigned int trampoline_tracker_free_by_context(void* context);

/**
 * Destroys a TTTracker by locating it through one of its trampolines.
 *
 * This convenience function finds the TTTracker that contains the specified
 * trampoline, then destroys the entire tracker and all its associated trampolines.
 * This allows cleanup without needing to know the tracker's context.
 *
 * @param trampoline A trampoline pointer that belongs to the tracker to be destroyed.
 *                   If NULL or if the trampoline is not found in any tracker,
 *                   the function returns 0.
 *
 * @return The number of trampolines that were freed from the destroyed tracker,
 *         or 0 if no tracker was found, if the trampoline wasn't found, or if
 *         the tracker was the global static tracker.
 *
 * @note This function will free ALL trampolines in the tracker that contains
 *       the specified trampoline, not just the specified trampoline itself.
 *
 * @warning After calling this function, the specified trampoline pointer and
 *          all other trampoline pointers from the same tracker become invalid.
 */
unsigned int trampoline_tracker_free_by_trampoline(void* trampoline);

/**
 * Tracks a trampoline allocation within a specific context.
 *
 * This function creates or finds a TTTracker for the given context and
 * adds the trampoline to its allocation list. If the context doesn't
 * exist, a new TTTracker is created and added to the global list.
 *
 * @param trampoline The trampoline pointer to track. If NULL and a
 * matching context exists, increments the failure count and returns NULL.
 *
 * @param context The context to associate with this trampoline. Multiple
 * trampolines can share the same context.
 *
 * @param tracker The TTTracker instance to use to track the trampoline
 * allocation and its associated context in memory. This helps for cleanup
 * and validation. If the tracker is NULL, a match will be attempted in
 * the global list before a new one is created to manage the context.
 *
 * @return A pointer to the TTTracker managing this trampoline, or NULL
 * if allocation failed or if trampoline was NULL with an existing context.
 *
 * @note The returned TTTracker should not be freed directly. Use the
 * appropriate cleanup functions instead.
 */
TTTracker* trampoline_track_with_tracker(
  void* trampoline,
  void* context,
  TTTracker* tracker
);

/**
 * Tracks a trampoline allocation within a specific context.
 *
 * This function creates or finds a TTTracker for the given context and
 * adds the trampoline to its allocation list. If the context doesn't
 * exist, a new TTTracker is created and added to the global list.
 *
 * @param trampoline The trampoline pointer to track. If NULL and a
 * matching context exists, increments the failure count and returns NULL.
 *
 * @param context The context to associate with this trampoline. Multiple
 * trampolines can share the same context.
 *
 * @return A pointer to the TTTracker managing this trampoline, or NULL
 * if allocation failed or if trampoline was NULL with an existing context.
 *
 * @note The returned TTTracker should not be freed directly. Use the
 * appropriate cleanup functions instead.
 */
TTTracker* trampoline_track(void* trampoline, void* context);

/**
 * Validates a TTTracker by checking for allocation failures.
 *
 * This function examines the failure count of the given tracker. If any
 * trampoline allocations failed (failure count > 0), it frees all
 * successfully allocated trampolines and returns 0. If all allocations
 * succeeded, it returns 1.
 *
 * @param tracker The TTTracker to validate. If NULL or if it's the global
 * static tracker, the function returns 1 (treating it as valid).
 *
 * @return 1 if the tracker has no failures (valid), or 0 if the tracker
 * had allocation failures (invalid). When 0 is returned, all trampolines
 * in the tracker have been freed.
 *
 * @note This function is typically called after a series of trampoline
 * allocations to ensure all succeeded before proceeding. If any failed,
 * it performs automatic cleanup.
 *
 * @warning If this function returns 0, the tracker and all its trampolines
 * have been freed and must not be used.
 */
int trampoline_validate(TTTracker* tracker);


/**
  Macro mandates

  Prefixes
    TDxx Trampoline Declarator
      Declarators are what you can use to help define the elements of a struct in a header

      struct Person {
        TDStringProperty(name, setName);

        char* _name; // see private structs below
      };

    TIxx Trampoline Implementor
    TI_xx Trampoline Implementor (Private Struct Overlap; see below)
      Implementors are helpers that define full functions to reduce boiler plate. In this
      case below, two functions are implemented. This macro is intended to be invoked in
      the header's matching .c file. The TIStringProperty macro creates a getter that
      returns the _name property as a const char*, and a setter that takes a const char*,
      frees _name if it is non-null, then allocates memory using calloc and strlen,
      finally strcpy'ing the supplied new value into _name. _name is left NULL if an error
      occurs.

      TIStringProperty(peep_name, peep_setName, Person, _name);
      TI_StringProperty(peep_name, peep_setName, Person, PrivatePerson, name);

    TAxx Trampoline Allocator
    TA_xx Trampoline Private Allocator
      Allocators are used in a struct new/make function to actually create and track the
      trampoline functions and assign them to the struct being created. In addition to
      tracking the potentially numerous trampolines in a given struct, allocators will
      perform the necessary allocation checking for a failed trampoline creation.

      Person* PersonMake() {
        TAAllocate(Person);
        // -> Person* public = calloc(1, sizeof(Person));
        // -> TTTracker *tracker = trampoline_track(public);

        TA_Allocate(Person, PrivatePerson);
        // -> PrivatePerson* private = calloc(1, sizeof(PrivatePerson));
        // -> Person* public = (Person*)private;
        // -> TTTracker *tracker = trampoline_track(private);

        if (public) {
          TAStringProperty(name, peep_name, setName, peep_setName, public);
          // -> peep->name = trampoline_create(peep_name, public);
          // -> peep->setName = trampoline_create(peep_setName, public);
        }

        // or
        if (private) {
          TA_StringProperty(name, peep_name, setName, peep_setName, public, private);
          // -> public->name = trampoline_create(peep_name, private);
          // -> public->setName = trampoline_create(peep_setName, private);
        }

        ...
        return peep;
      }


    TFxx Trampoline Function
    TF_xx Private Trampoline Functions (see below)
      Trampoline Functions are helper functions where the types and return values,
      are handled by the macro but the body is defined by the programmer. When defining
      unary, dyadic or other multi-parameter functions, it can be crucial to for the
      programmer to provide more complete implementations. See the documentation for
      each TFxxx trampoline function for provided args. Here's a unary example.

      TFUnary(void*, peep_next, void, Person) {
        // Perform code to find the next pointer and return it
      }

      Note that this generates code similar to the following

      void* peep_next(Person* self, void) {
        // Same implementation as above
      }

      Note the pointer to *self? This is the magic of trampoline functions. You
      declare the function pointer without the context/this pointer in the
      struct, but when the function is executed, the instance in question that
      was constructed at the time the trampoline was created is always received
      as the first parameter.

      Private Trampoline Functions have an underscore in the macro name. This
      allows some obfuscation in memory of where private variables are stored.
      So given the following code, macro variants exist to reduce boiler plate
      when the public and private structs overlap in memory.

      Generally the pattern here is to create a private variant of the struct
      when the call to create the public one is requested. Since these overlap
      and share the same memory, an ability to

      // In public header
      struct Person {
        TDUnary(void, processData, void*);
      };

      struct Person* PersonMake();

      // In private implementation file (.c)
      struct PrivatePerson {
        struct Person person;

        void* data;
      };

      TTF_Unary(void, per_processData, Person, PrivatePerson, void*, data)
        // Equates to
        //   void per_processData(Person* self, void* data) {
        //     PrivatePerson* private = (PrivatePerson*)self;

        // Implict scope is the `self`, `data` and `private`
        // Also note that in using the macro, I did not provide an opening brace
      }


    TTxx Trampoline Type
      These are types that help make the trampoline ecosystem work. In these cases, they
      might be structs to store previous allocations or paired information of a struct
      and its context. TTAllocNode or TTStructNode are trampoline types.


  In each of the above defined categories, xx indicates a suffix of some other type. For
  the available macros, here are some examples.

  xxGetter          -> a trampoline that returns a value, usually mated with a setter
  xxSetter          -> a trampoline that takes a new value, usually mated with a getter
  xxStringSetter    -> a specialized setter that frees and allocates c-strings wit calloc
  xxProperty        -> a trampoline pair that provides both a named getter and setter
  xxStringProperty  -> a trampoline pair centered around freeing and allocation a c-string

  xxNullary         -> a trampoline that has no parameters and no return type
  xxUnary           -> a trampoline that has a defined return and one parameter
  xxDyadic          -> a trampoline that has a defined return and two parameters
  xxTriadic         -> a trampoline that has a defined return and three parameters
  xxTetradic        -> a trampoline that has a defined return and four parameters
  xxPentadic        -> a trampoline that has a defined return and five parameters
  xxHexadic         -> a trampoline that has a defined return and six parameters
  */

// TDxx Trampoline Declarator (these are fine as-is)

#define TDGetter(getter, type) \
  type (*getter)(void);

#define TDStringGetter(getter) \
  const char* (*getter)(void);

#define TDSetter(setter, type) \
  void (*setter)(type);

#define TDStringSetter(setter) \
  void (*setter)(const char*);

#define TDProperty(getter, setter, type) \
  type (*getter)(void); \
  void (*setter)(type);

#define TDStringProperty(getter, setter) \
  const char* (*getter)(void); \
  void (*setter)(const char*);

#define TDNullary(nullary) \
  void (*nullary)(void);

#define TDUnary(return_type, unary, type) \
  return_type (*unary)(type);

#define TDDyadic(return_type, dyadic, type1, type2) \
  return_type (*dyadic)(type1, type2);

#define TDTriadic(return_type, triadic, type1, type2, type3) \
  return_type (*triadic)(type1, type2, type3);

#define TDTetradic(return_type, tetradic, type1, type2, type3, type4) \
  return_type (*tetradic)(type1, type2, type3, type4);

#define TDPentadic(return_type, pentadic, type1, type2, type3, type4, type5) \
  return_type (*pentadic)(type1, type2, type3, type4, type5);

#define TDHexadic(return_type, hexadic, type1, type2, type3, type4, type5, type6) \
  return_type (*hexadic)(type1, type2, type3, type4, type5, type6);

// TIxx Trampoline Implementor (corrected)

#define TIGetter(getter, context, type, variable_name) \
  type getter(context* self) { \
    return self->variable_name; \
  }

#define TI_Getter(getter, context, private_context, type, variable_name) \
  type getter(context* self) { \
    private_context* private = (private_context*)self; \
    return private->variable_name; \
  }

#define TISetter(setter, context, type, variable_name) \
  void setter(context* self, type newValue) { \
    self->variable_name = newValue; \
  }

#define TI_Setter(setter, context, private_context, type, variable_name) \
  void setter(context* self, type newValue) { \
    private_context* private = (private_context*)self; \
    private->variable_name = newValue; \
  }

#define TIStringSetter(setter, context, variable_name) \
  void setter(context* self, const char* newValue) { \
    if (self->variable_name) \
      free(self->variable_name); \
    \
    self->variable_name = calloc(1, strlen(newValue) + 1); \
    \
    if (self->variable_name) \
      strcpy(self->variable_name, newValue); \
  }

#define TI_StringSetter(setter, context, private_context, variable_name) \
  void setter(context* self, const char* newValue) { \
    private_context* private = (private_context*)self; \
    \
    if (private->variable_name) \
      free(private->variable_name); \
    \
    private->variable_name = calloc(1, strlen(newValue) + 1); \
    \
    if (private->variable_name) \
      strcpy(private->variable_name, newValue); \
  }

#define TIProperty(getter, setter, context, type, variable_name) \
  type getter(context* self) { \
    return self->variable_name; \
  } \
  void setter(context* self, type newValue) { \
    self->variable_name = newValue; \
  }

#define TI_Property(getter, setter, context, private_context, type, variable_name) \
  type getter(context* self) { \
    private_context* private = (private_context*)self; \
    return private->variable_name; \
  } \
  void setter(context* self, type newValue) { \
    private_context* private = (private_context*)self; \
    private->variable_name = newValue; \
  }

#define TIStringProperty(getter, setter, context, variable_name) \
  const char* getter(context* self) { \
    return self->variable_name; \
  } \
  void setter(context* self, const char* newValue) { \
    if (self->variable_name) \
      free(self->variable_name); \
    \
    self->variable_name = calloc(1, strlen(newValue) + 1); \
    \
    if (self->variable_name) \
      strcpy(self->variable_name, newValue); \
  }

#define TI_StringProperty(getter, setter, context, private_context, variable_name) \
  const char* getter(context* self) { \
    private_context* private = (private_context*)self; \
    return private->variable_name; \
  } \
  void setter(context* self, const char* newValue) { \
    private_context* private = (private_context*)self; \
    \
    if (private->variable_name) \
      free(private->variable_name); \
    \
    private->variable_name = calloc(1, strlen(newValue) + 1); \
    \
    if (private->variable_name) \
      strcpy(private->variable_name, newValue); \
  }

// TAxx Trampoline Allocator (corrected with error handling)

#define TAAllocate(public_struct) \
  public_struct* public = calloc(1, sizeof(public_struct)); \
  TTTracker* tracker = NULL;

#define TA_Allocate(public_struct, private_struct) \
  private_struct* private = calloc(1, sizeof(private_struct)); \
  public_struct* public = (public_struct*)private; \
  TTTracker* tracker = NULL;

// Helper macros for actually tracking trampolines during allocation
//
#ifdef HAS_PUBLIC_ARGC

#define TATrackTrampoline(trampoline_ptr, func_name, real_func, context, argc) \
  trampoline_ptr = trampoline_create(real_func, context, argc); \
  trampoline_track(trampoline_ptr, context);

#else

#define TATrackTrampoline(trampoline_ptr, func_name, real_func, context) \
  trampoline_ptr = trampoline_create(real_func, context); \
  trampoline_track(trampoline_ptr, context);

#endif


// TFxx Trampoline Function (corrected)

#define TFGetter(getter, context_type, return_type) \
  return_type getter(context_type* self)

#define TF_Getter(getter, context_type, private_context, return_type) \
  return_type getter(context_type* self) {\
    private_context* private = (private_context*)self;


#define TFSetter(setter, context_type, variable_type) \
  void setter(context_type* self, variable_type newValue)

#define TF_Setter(setter, context_type, private_context, variable_type) \
  void setter(context_type* self, variable_type newValue) {\
    private_context* private = (private_context*)self;


#define TFNullary(nullary, context_type) \
  void nullary(context_type* self)

#define TFUnary(\
  return_type, \
  unary, \
  context_type, \
  variable_type, \
  variable_name \
) \
  return_type unary(context_type* self, variable_type variable_name)

#define TFDyadic(\
  return_type, \
  dyadic, \
  context_type, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name \
) \
  return_type dyadic( \
    context_type* self, \
    variable1_type variable1_name, \
    variable2_type variable2_name \
  )

#define TFTriadic(\
  return_type, \
  triadic, \
  context_type, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name, \
  variable3_type, \
  variable3_name \
) \
  return_type triadic( \
    context_type* self, \
    variable1_type variable1_name, \
    variable2_type variable2_name, \
    variable3_type variable3_name \
  )

#define TFTetradic(\
  return_type, \
  tetradic, \
  context_type, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name, \
  variable3_type, \
  variable3_name, \
  variable4_type, \
  variable4_name \
) \
  return_type tetradic( \
    context_type* self, \
    variable1_type variable1_name, \
    variable2_type variable2_name, \
    variable3_type variable3_name, \
    variable4_type variable4_name \
  )

#define TFPentadic(\
  return_type, \
  pentadic, \
  context_type, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name, \
  variable3_type, \
  variable3_name, \
  variable4_type, \
  variable4_name, \
  variable5_type, \
  variable5_name \
) \
  return_type pentadic( \
    context_type* self, \
    variable1_type variable1_name, \
    variable2_type variable2_name, \
    variable3_type variable3_name, \
    variable4_type variable4_name, \
    variable5_type variable5_name \
  )

#define TFHexadic(\
  return_type, \
  hexadic, \
  context_type, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name, \
  variable3_type, \
  variable3_name, \
  variable4_type, \
  variable4_name, \
  variable5_type, \
  variable5_name, \
  variable6_type, \
  variable6_name \
) \
  return_type hexadic( \
    context_type* self, \
    variable1_type variable1_name, \
    variable2_type variable2_name, \
    variable3_type variable3_name, \
    variable4_type variable4_name, \
    variable5_type variable5_name, \
    variable6_type variable6_name \
  )

// Private variants with underscore (TF_xx)

#define TF_Nullary(nullary, public_context, private_context) \
  void nullary(public_context* self) { \
    private_context* private = (private_context*)self;

#define TF_Unary(\
  return_type, \
  unary, \
  public_context, \
  private_context, \
  variable_type, \
  variable_name \
) \
  return_type unary(public_context* self, variable_type variable_name) { \
    private_context* private = (private_context*)self;

#define TF_Dyadic(\
  return_type, \
  dyadic, \
  public_context, \
  private_context, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name \
) \
  return_type dyadic( \
    public_context* self, \
    variable1_type variable1_name, \
    variable2_type variable2_name \
  ) { \
    private_context* private = (private_context*)self;

#define TF_Triadic(\
  return_type, \
  triadic, \
  public_context, \
  private_context, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name, \
  variable3_type, \
  variable3_name \
) \
  return_type triadic( \
    public_context* self, \
    variable1_type variable1_name, \
    variable2_type variable2_name, \
    variable3_type variable3_name \
  ) { \
    private_context* private = (private_context*)self;

#define TF_Tetradic(\
  return_type, \
  tetradic, \
  public_context, \
  private_context, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name, \
  variable3_type, \
  variable3_name, \
  variable4_type, \
  variable4_name \
) \
  return_type tetradic( \
    public_context* self, \
    variable1_type variable1_name, \
    variable2_type variable2_name, \
    variable3_type variable3_name, \
    variable4_type variable4_name \
  ) { \
    private_context* private = (private_context*)self;

#define TF_Pentadic(\
  return_type, \
  pentadic, \
  public_context, \
  private_context, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name, \
  variable3_type, \
  variable3_name, \
  variable4_type, \
  variable4_name, \
  variable5_type, \
  variable5_name \
) \
  return_type pentadic( \
    public_context* self, \
    variable1_type variable1_name, \
    variable2_type variable2_name, \
    variable3_type variable3_name, \
    variable4_type variable4_name, \
    variable5_type variable5_name \
  ) { \
    private_context* private = (private_context*)self;

#define TF_Hexadic(\
  return_type, \
  hexadic, \
  public_context, \
  private_context, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name, \
  variable3_type, \
  variable3_name, \
  variable4_type, \
  variable4_name, \
  variable5_type, \
  variable5_name, \
  variable6_type, \
  variable6_name \
) \
  return_type hexadic( \
    public_context* self, \
    variable1_type variable1_name, \
    variable2_type variable2_name, \
    variable3_type variable3_name, \
    variable4_type variable4_name, \
    variable5_type variable5_name, \
    variable6_type variable6_name \
  ) { \
    private_context* private = (private_context*)self;

// Note: The TF_ variants intentionally leave the opening brace
// so the user can provide the implementation body

#ifdef __cplusplus
}
#endif
#endif /* TRAMPOLINE_H */
