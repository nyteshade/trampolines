#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // <-- FIX 1: Added for getpagesize()
#include "trampoline.h"

// The public-facing struct.
struct Person {
  char name[40];
  // This now correctly declares a pointer to a function that returns a 'const char *'.
  const char *(*getName)(); // <-- FIX 2: Corrected function pointer type
};

// The private implementation struct.
struct PersonImpl {
  struct Person person;
  // Add other private fields here if needed.
  int creation_id;
};

// The REAL implementation function. It follows the standard C convention
// of taking the context pointer as the first argument.
const char* _person_getName_impl(struct PersonImpl *self) {
    printf("(Inside C implementation, context is %p, id is %d)\n", self, self->creation_id);
    return self->person.name;
}

// "Constructor" for our Person class
struct Person *person_create(const char *name, int id) {
    struct PersonImpl *p_impl = malloc(sizeof(struct PersonImpl));
    if (!p_impl) return NULL;

    strncpy(p_impl->person.name, name, sizeof(p_impl->person.name) - 1);
    p_impl->person.name[sizeof(p_impl->person.name) - 1] = '\0';
    p_impl->creation_id = id;

    // Create the trampoline.
    // Bind the context 'p_impl' to the function '_person_getName_impl'.
    void* trampoline = trampoline_create(_person_getName_impl, p_impl);
    if (!trampoline) {
        free(p_impl);
        return NULL;
    }

    // The getName pointer now points to our executable trampoline code.
    p_impl->person.getName = trampoline;

    return &p_impl->person;
}

// "Destructor"
void person_free(struct Person *p) {
    if (!p) return;
    // We must free the trampoline's memory as well as the struct's.
    trampoline_free((void*)p->getName);
    // The Person pointer is part of the PersonImpl allocation, so we free the Impl.
    free(p);
}

int main() {
    printf("Creating Person objects with trampolines...\n\n");

    struct Person *p1 = person_create("Alice", 101);
    struct Person *p2 = person_create("Bob", 202);

    if (!p1 || !p2) {
        fprintf(stderr, "Failed to create person.\n");
        return 1;
    }

    printf("Calling p1->getName()... (Trampoline at %p)\n", p1->getName);
    const char *name1 = p1->getName(); // ✨ The magic call! ✨
    printf("  -> Result: %s\n\n", name1);

    printf("Calling p2->getName()... (Trampoline at %p)\n", p2->getName);
    const char *name2 = p2->getName(); // Each object has its own unique trampoline.
    printf("  -> Result: %s\n\n", name2);

    // Verify the trampolines are at different memory locations
    printf("p1 and p2 have different trampolines: %s\n",
           p1->getName == p2->getName ? "No" : "Yes");

    person_free(p1);
    person_free(p2);

    return 0;
}
