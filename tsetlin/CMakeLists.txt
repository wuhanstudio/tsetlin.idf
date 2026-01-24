# CMakeList.txt : CMake project for tsetlin.c, include source and define
# project specific logic here.
#


add_library(tsetlin STATIC
 "tsetlin.c" "tsetlin.h"
 "clause.h" "clause.c"
)

target_include_directories(tsetlin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(tsetlin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_include_directories(tsetlin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../protobuf)
target_include_directories(tsetlin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../random)
target_include_directories(tsetlin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../utils)
