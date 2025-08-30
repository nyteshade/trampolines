#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <exec/types.h>
#include <trampoline.h>
#include <trampoline_m68k_debug.h>

/* gcc -o trampoline main.c trampoline_m68k.c -Iram: -noixemul */

typedef struct AWindow {
  STRPTR title;

  const char *(*getTitle)();
  void        (*setTitle)(const char *newName);

  void        (*free)();
} AWindow;

struct AWindow* AWindowCreate(const char *title);

const char* _awin_getName(struct AWindow *self) {
  printf(
    "Inside _awin_getName(%p) -> title is %s (%p)\n",
    self,
    (self != NULL ? (char *)self->title : "null"),
    (void*)self->title
  );

  if (self && self->title) {
    return self->title;
  }

  return NULL;
}

void _awin_setName(struct AWindow *self, const char *newName) {
  printf("Inside _awin_setName(%p, \"%s\" (%p))\n", self, newName, (void*)newName);

  if (self) {
    if (self->title) {
      printf("Previous title %s (%p) existed, freeing...", self->title, self->title);
      free(self->title);
      printf("done\n");
      self->title = NULL;
    }

    printf("Allocating new string of length %d...", strlen(newName) + 1);
    self->title = (char*)calloc(1, strlen(newName) + 1);
    printf("done (%p)\n", self->title);

    if (self->title) {
      strcpy(self->title, newName);
      printf("New title set to %s (%p)\n", self->title, self->title);
    }
    else {
      self->title = NULL;
    }
  }
}

void _awin_free(struct AWindow *self) {
  if (self) {
    if (self->title) {
      free(self->title);
    }

    free(self);
  }
}

AWindow* AWindowCreate(const char *title) {
  AWindow* context = NULL;

  printf("Creating new AWindow...\n");
  printf("   ...allocating %d bytes\n", sizeof(AWindow));

  context = calloc(1, sizeof(AWindow));

  printf("   ...bytes allocated at %p\n", context);

  if (context) {
    context->getTitle = trampoline_create(_awin_getName, context);
    context->setTitle = trampoline_create(_awin_setName, context);
    context->free     = trampoline_create(_awin_free, context);

    printf("   ...trampoline getTitle is %p\n", context->getTitle);
    printf("   ...trampoline setTitle is %p\n", context->setTitle);
    printf("   ...trampoline free is %p\n", context->free);

    printf("   ...dumping setTitle trampoline\n\n");
    dump_tramp("setTitle", context->setTitle, 32);
    printf("\n");

    printf("   ...setting title to %s\n", title);

    context->setTitle(title);
  }
  else {
    context = NULL;
  }

  printf("...done. AWindow (%p) created.\n", context);
  return context;
}

int main() {
    AWindow* awin1 = NULL;
    AWindow* awin2 = NULL;

    const char* title1 = "Workbench1.3";
    const char* title2 = "Workbench2.0";

    printf("We will use %s (%p) for the first title\n", title1, title1);
    printf("We will use %s (%p) for the second title\n", title2, title2);

    printf("\nCreating first AWindow object\n");
    awin1 = AWindowCreate(title1);
    printf("Creating first AWindow object\n");
    awin2 = AWindowCreate(title2);

    if (!awin1 || !awin2) {
      if (awin1) awin1->free();
      if (awin2) awin2->free();

      return 1;
    }

    // Verify the trampolines are at different memory locations
    if (awin1) {
      printf("AWindow no.1 has the title -> %s\n", awin1->getTitle());
      awin1->free();
    }

    if (awin2) {
      printf("AWindow no.2 has the title -> %s\n", awin2->getTitle());
      awin2->free();
    }

    return 0;
}
