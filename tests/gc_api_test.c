/* Tests of the collector interface (spec, task 5.1, item 5).
 *
 * Usage: gc_api_test CASE
 * Each case exits with status 0 on success and prints the failed CHECK otherwise.
 * Heaps are deliberately small and every case allocates a lot of garbage, so the
 * collector has to run many times (or do many incremental steps) while the live
 * graph is being checked. */
#include "test_util.h"

static StellaPrimitive payload(StellaValue object) {
  return stella_object_read_primitive(object, 0);
}

/* 5.1.5: live data plus the requested object never exceed half of the limit,
 * while the program allocates many times the limit in total. */
static void run_half_limit(size_t max_heap_bytes) {
  /* A cell is 24 + 8*2 = 40 bytes; a junk object is 24 + 8*4 = 56 bytes. */
  size_t cells = (max_heap_bytes / 2 - 56) / 40;
  init_gc(max_heap_bytes);
  StellaValue list = NULL, cursor = NULL;
  stella_gc_push_root(&list);
  stella_gc_push_root(&cursor);

  for (size_t i = 1; i <= cells; ++i) {
    list = make_cell(&list, (StellaPrimitive)i);
    churn(NULL, 3);
  }
  churn(&list, 50 * max_heap_bytes / 56); /* ~50 heaps worth of garbage */

  StellaPrimitive sum = 0, count = 0;
  cursor = list;
  while (cursor != NULL) {
    sum += payload(cursor);
    ++count;
    read_into(&cursor, 0, &cursor);
  }
  CHECK(count == cells);
  CHECK(sum == cells * (cells + 1) / 2);

  stella_gc_pop_root(&cursor);
  stella_gc_pop_root(&list);
  stella_gc_shutdown();
}
static void test_half_limit_small(void) { run_half_limit(4096); }
static void test_half_limit_large(void) { run_half_limit(256 * 1024); }

/* ------------------------------------------------------------------------- */
struct test_case {
  const char *name;
  void (*run)(void);
};

static const struct test_case cases[] = {
    {"half_limit_small", test_half_limit_small},
    {"half_limit_large", test_half_limit_large},
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s CASE\n", argv[0]);
    return EXIT_FAILURE;
  }
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
    if (strcmp(argv[1], cases[i].name) == 0) {
      cases[i].run();
      return EXIT_SUCCESS;
    }
  }
  fprintf(stderr, "unknown test case: %s\n", argv[1]);
  return EXIT_FAILURE;
}
