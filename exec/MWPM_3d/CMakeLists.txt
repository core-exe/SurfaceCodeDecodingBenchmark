add_executable(MWPM_error_rate_3d MWPM_error_rate_3d.cpp)

target_link_libraries(MWPM_error_rate_3d PUBLIC
    error_dynamics
    decoder
    OpenMP::OpenMP_CXX
)