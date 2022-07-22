add_executable(MWPE_error_rate_2d MWPE_error_rate_2d.cpp)

target_link_libraries(MWPE_error_rate_2d PUBLIC
    error_dynamics
    decoder
    OpenMP::OpenMP_CXX
)