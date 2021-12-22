cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bazel2zephyr)

FILE(GLOB app_sources src/*.cpp)
target_sources(app PRIVATE ${app_sources})

set(LIB_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${LIB_DIR})

add_library(calculationMachine STATIC IMPORTED GLOBAL)
set_target_properties(calculationMachine PROPERTIES IMPORTED_LOCATION ${LIB_DIR}/libcalculationMachine.a)
target_link_libraries(app PUBLIC calculationMachine)