cmake_minimum_required(VERSION 3.19)
project(surface_code_decoding_benchmark)

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo"
        CACHE STRING
            "Build Type [Release, Debug, RelWithDebInfo]" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            Release RelWithDebInfo Debug)
endif ()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_CFLAGS} -g")
endif ()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_CFLAGS} -O3")
endif ()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_CFLAGS} -fopenmp -fvisibility=hidden")

find_package(OpenMP REQUIRED)

add_definitions(-DPROJECT_ROOT_PATH="${CMAKE_SOURCE_DIR}")

add_subdirectory(external/pybind11)
add_subdirectory(error_dynamics)
add_subdirectory(decoder)
add_subdirectory(example)
add_subdirectory(exec)