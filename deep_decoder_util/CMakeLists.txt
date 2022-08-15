cmake_minimum_required(VERSION 3.19)
project(deep_decoder_util)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pybind11_add_module(deep_decoder_util
    utility.hpp
    utility.cpp
)
target_link_libraries(deep_decoder_util PRIVATE
    error_dynamics
)

target_compile_definitions(deep_decoder_util
    PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO}
)
