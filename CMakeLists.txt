cmake_minimum_required(VERSION 3.10)
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

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_CFLAGS} -g")

add_subdirectory(error_dynamics)
add_subdirectory(example)