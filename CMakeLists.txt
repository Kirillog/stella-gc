cmake_minimum_required(VERSION 3.25)
project(stella_gc LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra)

# The collector: gc.cpp (C++23) once it exists, otherwise the C stub gc.c.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/gc.cpp)
  set(GC_SOURCE gc.cpp)
else()
  set(GC_SOURCE gc.c)
endif()

# Collector + runtime without the program entry point (runtime/main.c), so that
# C tests calling gc.h directly can provide their own main().
add_library(stella_gc STATIC
  ${GC_SOURCE}
  runtime/runtime.c
  runtime/scalar.c
)
target_include_directories(stella_gc PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})

# Each generated Stella program in examples/ becomes its own executable.
file(GLOB EXAMPLES CONFIGURE_DEPENDS examples/*.c)
foreach(example ${EXAMPLES})
  get_filename_component(name ${example} NAME_WE)
  add_executable(${name} ${example} runtime/main.c)
  target_link_libraries(${name} PRIVATE stella_gc)
  set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)
  install(TARGETS ${name})
endforeach()

enable_testing()
add_subdirectory(tests)
