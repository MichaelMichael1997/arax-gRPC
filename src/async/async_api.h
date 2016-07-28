#ifndef ASYNC_API_HEADER
#define ASYNC_API_HEADER
#include <stddef.h>

/**
 * Initialize a async_meta_s object once.
 *
 * This will be called only once, on the first node.
 *
 * @param meta An uninitialized async_meta_s object.
 */
void async_meta_init_once(async_meta_s * meta);

/**
 * Initialize a async_meta_s object on every node.
 *
 * This will be called multiple times, once for every node.
 *
 * @param meta An uninitialized async_meta_s object.
 */
void async_meta_init_always(async_meta_s * meta);

/**
 * Create and register async_completion_s objects created in \c buff.
 *
 * @param meta Pointer to async_meta_s that will 'own' this completion.
 * @param completion Completion to be initialized
 * @return Number of objects created, should be buff_size/async_completion_size().
 */
void async_completion_init(async_meta_s * meta,async_completion_s * completion);

/**
 * Mark \c compl as completed and notify pending async_completion_wait() callers.
 *
 * @param meta Pointer to async_meta_s used in async_completion_init.
 * @param completion Completion to be marked as completed.
 */
void async_completion_complete(async_meta_s * meta,async_completion_s * completion);

/**
 * Check if completion has been marked as completed.
 *
 * @param meta Pointer to async_meta_s used in async_completion_init.
 * @param completion Completion to be checked.
 * @return 0 if not completed, !0 if completed.
 */
int async_completion_check(async_meta_s * meta,async_completion_s * completion);

/**
 * Wait for \c compl to be completed with async_completion_complete().
 *
 * @param meta Pointer to async_meta_s used in async_completion_init.
 * @param completion Sleep untill it has been completed with async_completion_complete.
 */
void async_completion_wait(async_meta_s * meta,async_completion_s * completion);

/**
 * De initialize an async_meta_s object.
 *
 * @param meta The async_meta_s object to be uninitialized.
 */
void async_meta_exit(async_meta_s * meta);
#endif
