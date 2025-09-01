#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <trampoline.h>

typedef struct AWindow {
  char* title;
  int width;
  int height;

  const char *(*getTitle)();
  void        (*setTitle)(const char *newName);
  
  int         (*getWidth)();
  void        (*setWidth)(int newWidth);

  int         (*getHeight)();
  void        (*setHeight)(int newHeight);
  
  void        (*setSize)(int newWidth, int newHeight);

  void        (*free)();
} AWindow;


struct AWindow* AWindowCreate(const char *title);

/* use some macros to avoid unnecessary boiler plate */

TRAMP_GETTER(_awin_getName, AWindow, char*, title);
TRAMP_GETTER(_awin_getWidth, AWindow, int, width);
TRAMP_GETTER(_awin_getHeight, AWindow, int, height);

TRAMP_STRING_SETTER(_awin_setName, AWindow, title);
TRAMP_SETTER(_awin_setWidth, AWindow, int, width);
TRAMP_SETTER(_awin_setHeight, AWindow, int, height);

/* define more complex functions next */

void _awin_setSize(AWindow* self, int width, int height) {
  if (self) {
    self->setWidth(width);
    self->setHeight(height);
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

/* finally create the init function for our magic struct */

AWindow* AWindowCreate(const char *title) {
  AWindow* context = NULL;

  printf("Creating new AWindow...\n");
  printf("   ...allocating %d bytes\n", sizeof(AWindow));

  context = calloc(1, sizeof(AWindow));

  printf("   ...bytes allocated at %p\n", context);

  if (context) {
    trampoline_allocations allocations = { 0 };
    
    context->getTitle = trampoline_create_and_track(_awin_getName, context, 0, &allocations);
    context->setTitle = trampoline_create_and_track(_awin_setName, context, 1, &allocations);
    context->getWidth = trampoline_create_and_track(_awin_getWidth, context, 0, &allocations);
    context->setWidth = trampoline_create_and_track(_awin_setWidth, context, 1, &allocations);
    context->getHeight = trampoline_create_and_track(_awin_getHeight, context, 0, &allocations);
    context->setHeight = trampoline_create_and_track(_awin_setHeight, context, 1, &allocations);
    context->setSize = trampoline_create_and_track(_awin_setSize, context, 2, &allocations);
    context->free     = trampoline_create_and_track(_awin_free, context, 0, &allocations);

    printf("   ...trampoline getTitle is %p\n", context->getTitle);
    printf("   ...trampoline setTitle is %p\n", context->setTitle);
    printf("   ...trampoline getWidth is %p\n", context->getWidth);
    printf("   ...trampoline setWidth is %p\n", context->setWidth);
    printf("   ...trampoline getHeight is %p\n", context->getHeight);
    printf("   ...trampoline setHeight is %p\n", context->setHeight);
    printf("   ...trampoline setSize is %p\n", context->setSize);
    printf("   ...trampoline free is %p\n", context->free);
    
    if (trampolines_validate(&allocations)) {
      printf("   ...setting title to %s...", title);
      context->setTitle(title);
      printf("done\n");
      
      printf("   ...setting width to 640...");
      context->setWidth(640);
      printf("done\n");

      printf("   ...setting height to 480...");
      context->setHeight(480);
      printf("done\n");      
    }
    else {
      free(context);
      context = NULL;
    }
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
    printf("We will use 1024, 768 for the second window size\n");

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
      printf("  ... sized at %lux%lu pixels\n", awin1->getWidth(), awin1->getHeight());
      awin1->free();
    }

    if (awin2) {
      printf("(Adjusting second window size)\n");
      awin2->setSize(1024, 768);
      
      printf("AWindow no.2 has the title -> %s\n", awin2->getTitle());
      printf("  ... sized at %lux%lu pixels\n", awin2->getWidth(), awin2->getHeight());
      awin2->free();
    }

    return 0;
}
