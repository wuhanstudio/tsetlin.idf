# CMakeList.txt : CMake project for tsetlin.c, include source and define
# project specific logic here.
#


add_library(mnist STATIC
 "mnist.c" "mnist.h"
)

target_include_directories(mnist PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mnist PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../utils)