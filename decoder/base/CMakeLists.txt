add_library(decoder_base STATIC
    decoder_base.hpp
    decoder_base.cpp
)

target_link_libraries(decoder_base PUBLIC
    error_dynamics
)

target_include_directories(decoder_base PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")