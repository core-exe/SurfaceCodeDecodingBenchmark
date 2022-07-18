add_library(decoder STATIC
    decoder_base.hpp
    decoder_base.cpp
)

target_link_libraries(decoder PUBLIC
    error_dynamics
)

target_include_directories(decoder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")