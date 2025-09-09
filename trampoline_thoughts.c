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
     Implementors are helpers that define full functions to reduce boiler plate. In this
     case below, two functions are implemented. This macro is intended to be invoked in
     the header's matching .c file. The TIStringProperty macro creates a getter that
     returns the _name property as a const char*, and a setter that takes a const char*,
     frees _name if it is non-null, then allocates memory using calloc and strlen,
     finally strcpy'ing the supplied new value into _name. _name is left NULL if an error
     occurs.

      TIStringProperty(peep_name, peep_setName, Person, _name);

   TAxx Trampoline Allocator
     Allocators are used in a struct new/make function to actually create and track the
     trampoline functions and assign them to the struct being created. In addition to
     tracking the potentially numerous trampolines in a given struct, allocators will
     perform the necessary allocation checking for a failed trampoline creation.

      Person* PersonMake() {
        Person* peep = calloc(1, sizeof(Person));

        if (peep) {
          TAStringProperty(name, peep_name, setName, peep_setName, peep);
          ...
        }

        ...
        return peep;
      }

      turns TAStringProperty(...), roughly, into the following code:

        peep->name = trampoline_create(peep_name, peep);
        peep->setName = trampoline_create(peep_setName, peep);

   TFxx Trampoline Function
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

     Note the pointer to *self? This is the magic of trampoline functions. You declare
     the function pointer without the context/this pointer in the struct, but when the
     function is executed, the instance in question that was constructed at the time
     the trampoline was created is always received as the first parameter.

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

typedef struct TTAllocNode {
  struct TTAllocNode* next;
  void* trampoline;
} TTAllocNode;

typedef struct TTStructNode {
  struct TTStructNode* next;
  TTAllocNode* first;
  void* context;

  unsigned int failures;
  unsigned int count;
  unsigned long id;
} TTStructNode;

// TDxx Trampoline Declarator

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

// TIxx Trampoline Implementor

#define TIGetter(getter, context, type, variable_name) \
  type (*getter)(context* self) { return self->variable_name; };

#define TISetter(setter, context, type, variable_name) \
  void (*setter)(context* self, type newValue) { \
    self->variable_name = newValue; \
  };

#define TIStringSetter(setter, context, variable_name) \
  void (*setter)(context* self, const char* newValue) { \
    if (self->variable_name) \
      free(self->variable_name); \
    \
    self->variable_name = calloc(1, strlen(newValue) + 1); \
    \
    if (self->variable_name) \
      strcpy(self->variable_name, newValue); \
    \
    return self->variable_name; \
  };

#define TIProperty(getter, setter, context, type, variable_name) \
  type (*getter)(context* self) { return self->variable_name; } \
  void (*setter)(context* self, type newValue) { \
    self->variable_name = newValue; \
  }

#define TTStringProperty(getter, setter, context, variable_name) \
  type (*getter)(context* self) { return self->variable_name; } \
  void (*setter)(context* self, const char* newValue) { \
    if (self->variable_name) \
      free(self->variable_name); \
    \
    self->variable_name = calloc(1, strlen(newValue) + 1); \
    \
    if (self->variable_name) \
      strcpy(self->variable_name, newValue); \
    \
    return self->variable_name; \
  };

// TAxx Trampoline Allocator

// TFxx Trampoline Function

// TTxx Trampoline Type
