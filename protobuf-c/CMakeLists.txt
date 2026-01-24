# CMakeList.txt : CMake project for tsetlin.c, include source and define
# project specific logic here.
#


add_library(protobuf STATIC
 "protobuf-c.c" "protobuf-c.h"
)

target_include_directories(protobuf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})