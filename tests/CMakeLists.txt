# Tests for the collector.  Run from the build directory:  ctest --output-on-failure
#
# The tests run against a collector build with the smallest incremental step
# (GC_SCAN_COUNT=1), which exercises long cycles spread over many steps.
add_library(stella_gc_step1 STATIC
  ${PROJECT_SOURCE_DIR}/${GC_SOURCE}
  ${PROJECT_SOURCE_DIR}/runtime/runtime.c
  ${PROJECT_SOURCE_DIR}/runtime/scalar.c)
target_include_directories(stella_gc_step1 PUBLIC
  ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR})
target_compile_definitions(stella_gc_step1 PRIVATE GC_SCAN_COUNT=1)

add_executable(gc_api_test_step1 gc_api_test.c)
target_link_libraries(gc_api_test_step1 PRIVATE stella_gc_step1)
set_target_properties(gc_api_test_step1 PROPERTIES LINKER_LANGUAGE CXX)

foreach(case half_limit_small half_limit_large)
  add_test(NAME api.step1.${case} COMMAND gc_api_test_step1 ${case})
  set_tests_properties(api.step1.${case} PROPERTIES TIMEOUT 60)
endforeach()
