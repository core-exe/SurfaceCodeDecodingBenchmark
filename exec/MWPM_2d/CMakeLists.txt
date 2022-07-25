add_executable(MWPM_error_rate_2d MWPM_error_rate_2d.cpp)

target_link_libraries(MWPM_error_rate_2d PUBLIC
    error_dynamics
    decoder
    OpenMP::OpenMP_CXX
)