#include "trampoline.h"
#include <stdlib.h>

TTTracker __trampolines = { 0 };

TTTracker* trampoline_find_matching_context(void* context) {
  TTTracker* next = &__trampolines;

  for (; next; next = next->next) {
    if (next->context == context)
      break;
  }

  return next;
}

TTTracker* find_tracker_for_trampoline(void* trampoline) {
  TTTracker* tracker = &__trampolines;

  /* Iterate through all trackers in the global list */
  while (tracker) {
    /* Skip trackers with no allocation nodes */
    if (tracker->first) {
      TTAllocNode* node = tracker->first;

      /* Search through all allocation nodes in this tracker */
      while (node) {
        if (node->trampoline == trampoline)
          return tracker;

        node = node->next;
      }
    }

    tracker = tracker->next;
  }

  return NULL;
}

TTTracker* trampoline_track_with_tracker(
  void* trampoline,
  void* context,
  TTTracker* tracker
) {
  TTAllocNode* node = calloc(1, sizeof(TTAllocNode));
  TTAllocNode* last = NULL;
  TTTracker* parent = tracker;
  TTTracker* lastParent = NULL;

  if (parent == NULL) {
    /* Make an effort to find a match if we weren't given one */
    parent = trampoline_find_matching_context(context);
  }

  if (!trampoline && parent) {
    parent->failures++;
    return NULL;
  }

  if (!node)
    return NULL;

  /* If the parent is null, meaning we couldn't find one, create a new one
   * which means also adding the new one to the end of the list.
   */
  if (!parent) {
    parent = calloc(1, sizeof(TTTracker));

    /* If we failed to create a new parent, free the alloc node and quit */
    if (!parent) {
      free(node);
      return NULL;
    }

    parent->context = context;

    /* Move a pointer to the last TTTracker in the list */
    for (
      lastParent = &__trampolines;
      lastParent && lastParent->next;
      lastParent = lastParent->next
    );

    /* Assign our new parent to the last parent's next pointer */
    lastParent->next = parent;
  }

  /* Assign our trampoline to our new AllocNode  */
  node->trampoline = trampoline;
  node->next = NULL;

  if (parent->first == NULL) {
    parent->first = node;
    parent->count++;
    return parent;
  }

  /* Still here? Find the last AllocNode so we can append a new one */
  for (last = parent->first; last && last->next; last = last->next);

  parent->count++;
  last->next = node;

  return parent;
}

TTTracker* trampoline_track(void* trampoline, void* context) {
  TTTracker* parent = trampoline_find_matching_context(context);
  return trampoline_track_with_tracker(trampoline, context, parent);
}

unsigned int trampoline_tracker_free(TTTracker* tracker) {
  TTTracker* prev = NULL;
  TTAllocNode* node = NULL;
  TTAllocNode* next_node = NULL;
  unsigned int freed_count = 0;

  /* If no tracker found for this context, nothing to do */
  if (!tracker)
    return 0;

  /* First, free all trampolines and their nodes */
  node = tracker->first;
  while (node) {
    next_node = node->next;

    /* Free the trampoline itself */
    if (node->trampoline) {
      trampoline_free(node->trampoline);
      freed_count++;
    }

    /* Free the allocation node */
    free(node);

    node = next_node;
  }

  /* Now find the previous tracker in the global list so we can unlink */
  prev = &__trampolines;
  while (prev && prev->next != tracker) {
    prev = prev->next;
  }

  /* Unlink the tracker from the global list */
  if (prev)
    prev->next = tracker->next;

  /* Finally, free the tracker itself */
  /* BUT Don't try to free the global static tracker */
  if (tracker != &__trampolines)
    free(tracker);

  return freed_count;
}

unsigned int trampoline_tracker_free_by_context(void* context) {
  TTTracker* tracker = trampoline_find_matching_context(context);

  return trampoline_tracker_free(tracker);
}

unsigned int trampoline_tracker_free_by_trampoline(void* trampoline) {
  TTTracker* tracker = NULL;

  /* Find the tracker that contains this trampoline */
  tracker = find_tracker_for_trampoline(trampoline);

  /* If no tracker found, nothing to destroy */
  if (!tracker)
    return 0;

  /* Don't try to destroy the global static tracker */
  if (tracker == &__trampolines)
    return 0;

  /* Destroy the tracker directly */
  return trampoline_tracker_free(tracker);  // BUG FIX: Was tracker->context
}

int trampoline_validate(TTTracker* tracker) {
  /* NULL tracker or global tracker is considered valid */
  if (!tracker || tracker == &__trampolines)
    return 1;

  /* Check if there were any allocation failures */
  if (tracker->failures > 0) {
    /* Had failures - free all successful allocations and the tracker */
    trampoline_tracker_free(tracker);
    return 0;
  }

  /* No failures - tracker is valid */
  return 1;
}

#ifdef NO_PUBLIC_ARGC
  void *trampoline_monitor(
    void* target_func,
    void* context,
    TTTracker** tracker
  ) {
    void* trampoline = trampoline_create(target_func, context);
    *tracker = trampoline_track_with_tracker(trampoline, context, *tracker);

    return trampoline;
  }
#else
  void *trampoline_monitor(
    void* target_func,
    void* context,
    size_t public_argc,
    TTTracker** tracker
  ) {
    void* trampoline = trampoline_create(target_func, context, public_argc);
    *tracker = trampoline_track_with_tracker(trampoline, context, *tracker);

    return trampoline;
  }
#endif
