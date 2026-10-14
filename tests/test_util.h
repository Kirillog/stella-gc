/* Shared helpers for C tests that call the collector interface (gc.h) directly.
 *
 * Every test follows the ABI discipline that generated code follows:
 *   - every value returned by a collection point is stored into a registered root
 *     before the next collection point;
 *   - objects are passed to read/write/update by the address of a registered root;
 *   - fields are initialised right after stella_gc_alloc, with no collection point in between.
 */
#ifndef STELLA_GC_TEST_UTIL_H
#define STELLA_GC_TEST_UTIL_H

#include <stella/gc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__)
#define TEST_UNUSED __attribute__((unused))
#else
#define TEST_UNUSED
#endif

#define CHECK(condition)                                                     \
  do {                                                                       \
    if (!(condition)) {                                                      \
      fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__,       \
              #condition);                                                   \
      exit(EXIT_FAILURE);                                                    \
    }                                                                        \
  } while (0)

#define DATA_DESCRIPTOR(m, p, c, name)                                       \
  {STELLA_C_GC_ABI_VERSION, STELLA_OBJECT_DATA, 0, 0, (m), (p), (c), NULL,   \
   (name)}

/* A cons-like cell: one managed link and one primitive payload. */
static const StellaObjectDescriptor TEST_UNUSED test_cell = DATA_DESCRIPTOR(1, 1, 2, "test.cell");
/* Garbage object: one managed field (points into live data) and padding. */
static const StellaObjectDescriptor TEST_UNUSED test_junk = DATA_DESCRIPTOR(1, 1, 4, "test.junk");

static inline void init_gc(size_t max_heap_bytes) {
  StellaGcConfig config = {STELLA_C_GC_ABI_VERSION, max_heap_bytes};
  stella_gc_init(&config);
}

/* Allocates a cell (next, payload). *next_root must be a registered root. */
static inline StellaValue make_cell(StellaValue *next_root, StellaPrimitive payload) {
  StellaValue cell = stella_gc_alloc(&test_cell);
  stella_object_init_managed(cell, 0, *next_root);
  stella_object_init_primitive(cell, 0, payload);
  return cell;
}

/* Allocates `count` unreachable objects.  Each of them points to *target_root
 * (if the root is not NULL) so that the collector has to deal with pointers
 * from garbage into live data.  With a small heap this forces many collections
 * or incremental steps. */
static inline void churn(StellaValue *target_root, size_t count) {
  StellaValue junk = NULL;
  stella_gc_push_root(&junk);
  for (size_t i = 0; i < count; ++i) {
    junk = NULL; /* the previous junk object must not stay live during alloc */
    junk = stella_gc_alloc(&test_junk);
    stella_object_init_managed(junk, 0, target_root == NULL ? NULL : *target_root);
    stella_object_init_primitive(junk, 0, (StellaPrimitive)i);
  }
  stella_gc_pop_root(&junk);
}

/* Reads a managed field into *out_root (a registered root). */
static inline void read_into(StellaValue *object_root, size_t index, StellaValue *out_root) {
  StellaValue value = stella_gc_read_managed(object_root, index);
  *out_root = value;
}

#endif
