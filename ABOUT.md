# Trampolines

## Overview 

This project provides a way to create function pointers that receive
supplied context as their first parameter in the function definition 
but do not have that first parameter at their point of usage.

Essentially, the header defines a stuct that has a nested function
pointer. Typically when this pattern is followed, it forces the C 
programmer to also provide the struct instance itself as the first
parameter to each function pointer it has within in order to provide
what is effectively the `this` pointer. This pattern can be verbose
and feels like a waste of time.

**But, what if, like this example, the `append()` function had its 
own reference to self?**

```c
/* in header.h file */

typedef struct Array {
  void** items;
  size_t count;
  size_t capacity;

  void (*append)(void* element);
} Array;

Array* ArrayMake(size_t capacity, ...);
```

The devil is in the details, and more on that soon enough, but here
in the implementation, we see the function pointer being created using
this projects `trampoline_create()` function. You'll also note that the
private `array_append()` function receives a `Array* self` parameter
as its first argument?

**The struct doesn't show this, but the implementation has it. This is the
magic of the trampolines project**

```c
/* in implementation.c file */

void array_append(Array* self, size_t capacity, ...) {
  // magic is that self is automagically prepended to the arg stack
  // size_t is now the second parameter here
  // va_args elements to add
}

Array* ArrayMake(size_t capacity, ...) {
  Array* array = NULL;

  // ...

  array->append = trampoline_create(
    array_append, array, 1
  );

  return array;
}
```

## How it works

If you know a little assembly, this will be easy to grok. I don't know much
but directed several large language models to help me get the desired output
and then added the finishing touches that I wanted on top.

Basically many (all?) CPUs have data and address registers that hold numbers
and memory addresses, respectively. When you invoke a function, these registers
are filled with information relavant to the function, including its arguments.

Have you ever wondered what makes a `void (*doit)()` pointer executable but
a `char*` pointer is not? I did, and that's partially where the impetus for
this project came from.

A trampoline function uses some cpu specific, and sometimes OS specific, 
assembly instructions take a stored pointer reference to the struct (in the
example above, but it could be anything) and it effectively inserts it into
argument registers before calling the function that receives it.

So when the `trampoline_create()` function is invoke, it takes two or three,
depending on the cpu architecture, parameters. The first is the pointer to 
the function to call after argument stack is modified, the second is a pointer
to the object you want the function to receive as its first parameter; in our
case, a pointer to the newly created Array instance, and lastly, in some cases,
the number of publicly visible arguments. In the trampoline function described
in the header, `void (*append)(void* element)` receives one parameter. So we
create the trampoline with a single public argument.

## Caveats

This is CPU (and sometimes OS) specific as mentioned before. So an implementation
is required for each, but the same header is sufficient. On macOS this is greatly
simplified because a multi-architecture `libtrampoline.a` or `libtrampoline.dylib`
can be created. 

On all other platforms you would either create a library as needed or you would
add the trampoline.h file and the appropriate cpu architecture C file. There is
also a helper C file that is agnostic to CPU. So you might build this into your 
project using something like this

```sh
gcc \
  -I/path/to/your/project/includes \
  -o executable \
  main.c \
  trampoline_x86_64.c \
  trampoline_helpers.c
```

You must `cp trampoline.h /path/to/your/project/includes` in order for your code
to find the `#include <trampoline.h>` header. Then you can create your code as 
you see fit. Swap out `trampoline_x86_64.c` for `trampoline_ppc.c` if you're 
building this on a PowerMac or `trampoline_m68k.c` if you're building this on an
Amiga.

## Platform Support

Tested working platforms are:
 * m68k (680x0 Commodore Amigas). Tested compilers are gcc 2.95.3 and SAS/C 6.58
 * ppc (32-bit PowerPC). Tested on Mac OS X Panther, Mac OS X Tiger, Mac OS X Leopard
 * x86_64 (MacOS). Tested on various intel variants of macOS. Should work on Linux
 * arm64 (MacOS M-series CPUs)

Untested but should work:
 * x86_64 Linux
 * arm64 (Non-apple *nix variants)

In flight work to support the folowing platforms:
 * arm32 (Rasperry Pi and 32-bit ARM)(ready to test)
 * i386 (32-bit Intel)(tested - still doesn't work)
 * i386 (Windows specific 32-bit Intel)(probably doesn't work)
 * x86_64 (Windows specific 64-bit Intel)(ready to test)
 * ppc64 (G5 64-bit)

## Helpers

Several macros and functions are being considered to make developing object 
oriented programming with structs more convenient. The goal is to maximize
capability and ease of programming and reduce the number of compiled files 
needed. So currently there is a focus on C based macros. This part is in 
flight

Existing:
 * TRAMP_GETTER
 * TRAMP_GETTER_ (retrieves content from private wrapper struct)
 * TRAMP_SETTER
 * TRAMP_STRING_SETTER (frees the storage and strcpy's the new value in place)
 * TRAMP_FREE_SETTER (frees the storage before setting new content)

Planned:
 * TRAMP_SETTER_ (same as others but uses private wrapper struct)
 * TRAMP_STRING_SETTER_ 
 * TRAMP_FREE_SETTER_
 * TRAMP_PROPERTY
 * TRAMP_PROPERTY_
