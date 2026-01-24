# CMakeList.txt : CMake project for tsetlin.c, include source and define
# project specific logic here.
#


add_library(random STATIC
 "pcg32_fast.c" "xorshift128.c"
 "fast_rand_seed.h" "fast_rand.h"
)

target_include_directories(random PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})