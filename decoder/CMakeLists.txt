add_subdirectory(base)
add_subdirectory(matching)

add_library(decoder STATIC
    decoder.cpp
    decoder.hpp
)

target_link_libraries(decoder PUBLIC
    decoder_base
    matching_decoder
)

target_include_directories(decoder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
