# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("lime-tm")

# Require a modern C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include sub-projects.
set(TSETLIN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../tsetlin)
set(TSETLIN_BINARY_DIR ${CMAKE_BINARY_DIR}/tsetlin)
add_subdirectory(${TSETLIN_SOURCE_DIR} ${TSETLIN_BINARY_DIR})

set(RANDOM_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../random)
set(RANDOM_BINARY_DIR ${CMAKE_BINARY_DIR}/random)
add_subdirectory(${RANDOM_SOURCE_DIR} ${RANDOM_BINARY_DIR})

set(PROTOBUF_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../protobuf)
set(PROTOBUF_BINARY_DIR ${CMAKE_BINARY_DIR}/protobuf)
add_subdirectory(${PROTOBUF_SOURCE_DIR} ${PROTOBUF_BINARY_DIR})

set(PROTOBUF_C_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../protobuf-c)
set(PROTOBUF_C_BINARY_DIR ${CMAKE_BINARY_DIR}/protobuf-c)
add_subdirectory(${PROTOBUF_C_SOURCE_DIR} ${PROTOBUF_C_BINARY_DIR})

set(MNIST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../mnist)
set(MNIST_BINARY_DIR ${CMAKE_BINARY_DIR}/mnist)
add_subdirectory(${MNIST_SOURCE_DIR} ${MNIST_BINARY_DIR})

add_executable(lime-tm "main.c")

if (MSVC)
    set_source_files_properties(
        ${PROTOBUF_SOURCE_DIR}/tsetlin.pb-c.c
        PROPERTIES
        LANGUAGE CXX
    )
endif()

# Set the default startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT lime-tm)

# List of libraries
set(MAIN_LIBS 
    random
    tsetlin
    tsetlin-pb
)

# Cross-platform math library linking
if (UNIX)
    # On Linux/macOS, link libm for math functions
    list(APPEND MAIN_LIBS m)
endif()

# On Windows (MSVC), math functions are already in the runtime, no extra linking needed

target_include_directories(lime-tm
    PUBLIC ${CMAKE_SOURCE_DIR}/../../utils
)

# Link to the subfolder libraries
target_link_libraries(lime-tm 
	PRIVATE ${MAIN_LIBS}
    PRIVATE mnist
)

configure_file(${CMAKE_SOURCE_DIR}/tsetlin_model_8_bit.cpb
               ${CMAKE_BINARY_DIR}/mnist/tsetlin_model_8_bit.cpb
               COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/mnist/train-images-idx3-ubyte
               ${CMAKE_BINARY_DIR}/mnist/train-images-idx3-ubyte
               COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/mnist/train-labels-idx1-ubyte
               ${CMAKE_BINARY_DIR}/mnist/train-labels-idx1-ubyte
               COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/mnist/t10k-images-idx3-ubyte
               ${CMAKE_BINARY_DIR}/mnist/t10k-images-idx3-ubyte
               COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/mnist/t10k-labels-idx1-ubyte
               ${CMAKE_BINARY_DIR}/mnist/t10k-labels-idx1-ubyte
               COPYONLY)
