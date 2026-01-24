# CMakeList.txt : CMake project for tsetlin.c, include source and define
# project specific logic here.
#


add_library(mnist STATIC
 "mnist.c" "mnist.h"
)

# Cross-platform math library linking
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
   CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # On Linux/macOS, link libm for math functions
    target_link_libraries(mnist
        PRIVATE m
    )
endif()

target_include_directories(mnist PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mnist PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../utils)