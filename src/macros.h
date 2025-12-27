#ifndef TRAMPOLINE_MACROS_H
#define TRAMPOLINE_MACROS_H

#include <trampoline/trampoline.h>

#ifdef __cplusplus
extern "C" {
#endif

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
  type (*getter)(void)

#define TDStringGetter(getter) \
  const char* (*getter)(void)

#define TDSetter(setter, type) \
  void (*setter)(type)

#define TDStringSetter(setter) \
  void (*setter)(const char*)

#define TDProperty(getter, setter, type) \
  type (*getter)(void); \
  void (*setter)(type)

#define TDStringProperty(getter, setter) \
  const char* (*getter)(void); \
  void (*setter)(const char*)

#define TDNullary(nullary) \
  void (*nullary)(void)

#define TDUnary(return_type, unary, type) \
  return_type (*unary)(type)

#define TDDyadic(return_type, dyadic, type1, type2) \
  return_type (*dyadic)(type1, type2)

#define TDTriadic(return_type, triadic, type1, type2, type3) \
  return_type (*triadic)(type1, type2, type3)

#define TDTetradic(return_type, tetradic, type1, type2, type3, type4) \
  return_type (*tetradic)(type1, type2, type3, type4)

#define TDPentadic(return_type, pentadic, type1, type2, type3, type4, type5) \
  return_type (*pentadic)(type1, type2, type3, type4, type5)

#define TDHexadic(return_type, hexadic, type1, type2, type3, type4, type5, type6) \
  return_type (*hexadic)(type1, type2, type3, type4, type5, type6)

#define TDVoidFunc(fnName) \
  TDNullary(fnName)

#define TD1ArgFunc(return_type, fnName, type) \
  TDUnary(return_type, fnName, type)

#define TD2ArgFunc(return_type, dyadic, type1, type2) \
  TDDyadic(return_type, dyadic, type1, type2)

#define TD3ArgFunc(return_type, fnName, type1, type2, type3) \
  TDTriadic(return_type, fnName, type1, type2, type3)

#define TD4ArgFunc(return_type, fnName, type1, type2, type3, type4) \
  TDTetradic(return_type, fnName, type1, type2, type3, type4)

#define TD5ArgFunc(return_type, fnName, type1, type2, type3, type4, type5) \
  TDPentadic(return_type, fnName, type1, type2, type3, type4, type5)

#define TD6ArgFunc(return_type, fnName, type1, type2, type3, type4, type5, type6) \
  TDHexadic(return_type, fnName, type1, type2, type3, type4, type5, type6)

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
  TTTracker* tracker = NULL

#define TA_Allocate(public_struct, private_struct) \
  private_struct* private = calloc(1, sizeof(private_struct)); \
  public_struct* public = (public_struct*)private; \
  TTTracker* tracker = NULL

// Helper macros for actually tracking trampolines during allocation
//
#ifdef HAS_PUBLIC_ARGC

#define TATrackTrampoline(trampoline_ptr, func_name, real_func, context, argc) \
  trampoline_ptr = trampoline_create(real_func, context, argc); \
  trampoline_track(trampoline_ptr, context)

#define TAFunction(public_fn, impl_fn, argc) \
  public->public_fn = trampoline_monitor(impl_fn, public, argc, &tracker)

#define TAGetter(public_fn, impl_fn) \
  public->public_fn = trampoline_monitor(impl_fn, public, 0, &tracker)

#define TASetter(public_fn, impl_fn) \
  public->public_fn = trampoline_monitor(impl_fn, public, 1, &tracker)

#define TAProperty(public_getter, public_setter, impl_getter, impl_setter) \
  public->public_getter = trampoline_monitor(impl_getter, public, 0, &tracker); \
  public->public_setter = trampoline_monitor(impl_setter, public, 1, &tracker)

#else

#define TAFunction(public_fn, impl_fn, ignored_argc) \
  public->public_fn = trampoline_monitor(impl_fn, public, &tracker)

#define TAGetter(public_fn, impl_fn) \
  public->public_fn = trampoline_monitor(impl_fn, public, &tracker)

#define TASetter(public_fn, impl_fn) \
  public->public_fn = trampoline_monitor(impl_fn, public, &tracker)

#define TAProperty(public_getter, public_setter, impl_getter, impl_setter) \
  public->public_getter = trampoline_monitor(impl_getter, public, &tracker); \
  public->public_setter = trampoline_monitor(impl_setter, public, &tracker)

#define TATrackTrampoline(trampoline_ptr, func_name, real_func, context, ignored_argc) \
  trampoline_ptr = trampoline_create(real_func, context); \
  trampoline_track(trampoline_ptr, context)

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

#define TFVoidFunc(nullary, context_type) \
  TFNullary(nullary, context_type)

#define TF1ArgFunc(\
  return_type, \
  unary, \
  context_type, \
  variable_type, \
  variable_name \
) \
  TFUnary(\
    return_type, \
    unary, \
    context_type, \
    variable_type, \
    variable_name \
  )

#define TF2ArgFunc(\
  return_type, \
  dyadic, \
  context_type, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name \
) \
  TFDyadic(\
    return_type, \
    dyadic, \
    context_type, \
    variable1_type, \
    variable1_name, \
    variable2_type, \
    variable2_name \
  )

#define TF3ArgFunc(\
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
  TFTriadic(\
    return_type, \
    triadic, \
    context_type, \
    variable1_type, \
    variable1_name, \
    variable2_type, \
    variable2_name, \
    variable3_type, \
    variable3_name \
  )

#define TF4ArgFunc(\
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
  TFTetradic(\
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
  )

#define TF5ArgFunc(\
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
  TFPentadic(\
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
  )

#define TF6ArgFunc(\
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
  TFHexadic(\
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

#define TF_VoidFunc(nullary, public_context, private_context) \
  TF_Nullary(nullary, public_context, private_context)

#define TF_1ArgFunc(\
  return_type, \
  unary, \
  public_context, \
  private_context, \
  variable_type, \
  variable_name \
) \
  TF_Unary(\
    return_type, \
    unary, \
    public_context, \
    private_context, \
    variable_type, \
    variable_name \
  )

#define TF_2ArgFunc(\
  return_type, \
  dyadic, \
  public_context, \
  private_context, \
  variable1_type, \
  variable1_name, \
  variable2_type, \
  variable2_name \
) \
  TF_Dyadic(\
    return_type, \
    dyadic, \
    public_context, \
    private_context, \
    variable1_type, \
    variable1_name, \
    variable2_type, \
    variable2_name \
  )

#define TF_3ArgFunc(\
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
  TF_Triadic(\
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
  )

#define TF_4ArgFunc(\
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
  TF_Tetradic(\
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
  )

#define TF_5ArgFunc(\
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
  TF_Pentadic(\
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
  )

#define TF_6ArgFunc(\
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
  TF_Hexadic(\
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
  )

// Note: The TF_ variants intentionally leave the opening brace
// so the user can provide the implementation body

#ifdef __cplusplus
}
#endif
#endif /* TRAMPOLINE_MACROS_H */
