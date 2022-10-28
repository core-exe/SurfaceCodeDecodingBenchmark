add_executable(IteratedDecoder_error_rate_2d IteratedDecoder_error_rate_2d.cpp)

target_link_libraries(IteratedDecoder_error_rate_2d PUBLIC
    error_dynamics
    decoder
    OpenMP::OpenMP_CXX
)