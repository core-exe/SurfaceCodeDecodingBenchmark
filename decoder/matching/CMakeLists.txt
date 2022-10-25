add_subdirectory(Minimum-Cost-Perfect-Matching)

add_library(matching_decoder STATIC
    matching_util.cpp
    matching_util.hpp
    matching_decoder.hpp
    simple_matching_decoder.hpp
    simple_matching_decoder.cpp
    iterated_decoder.hpp
    iterated_decoder.cpp
)

target_link_libraries(matching_decoder PUBLIC
    minimum_cost_perfect_matching
    error_dynamics
    decoder_base
)

target_include_directories(matching_decoder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
