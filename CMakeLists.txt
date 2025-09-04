cmake_minimum_required(VERSION 3.40)
project(junco)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory("${CMAKE_SOURCE_DIR}/src/")

option(BUILD_TESTS "Whether tests (from ./testing/) should be built." true)
if (BUILD_TESTS)
    add_subdirectory("${CMAKE_SOURCE_DIR}/testing/")
endif()