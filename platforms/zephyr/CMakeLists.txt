cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_tsetlin)

target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../tsetlin)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../protobuf)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../random)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../mnist)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../utils)

file(GLOB app_sources src/* "../../tsetlin/*" "../../protobuf/*" "../../protobuf-c/*" "../../mnist/*" "../../random/*")
target_sources(app PRIVATE ${app_sources})
