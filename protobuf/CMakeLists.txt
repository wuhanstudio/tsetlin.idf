# CMakeList.txt : CMake project for tsetlin.c, include source and define
# project specific logic here.
#


add_library(tsetlin-pb STATIC
 "tsetlin.pb-c.c" "tsetlin.pb-c.h"
)

target_include_directories(tsetlin-pb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tsetlin-pb
    PUBLIC protobuf
)

target_include_directories(tsetlin-pb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
