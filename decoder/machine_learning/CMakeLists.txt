add_library(machine_learning_decoder STATIC
    ml_decoder.hpp
    ml_decoder.cpp
    machine_learning.hpp
)

target_link_libraries(machine_learning_decoder PUBLIC
    error_dynamics
    decoder_base
    pybind11::embed
)

file(REAL_PATH "." DEEP_DECODER_PY_PKG_PATH)
add_compile_definitions(DEEP_DECODER_PY_PKG_PATH="${DEEP_DECODER_PY_PKG_PATH}")

target_include_directories(machine_learning_decoder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

