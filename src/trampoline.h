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


#ifdef __cplusplus
}
#endif
#endif /* TRAMPOLINE_H */
