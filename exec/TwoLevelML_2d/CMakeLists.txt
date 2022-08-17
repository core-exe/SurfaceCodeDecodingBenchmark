add_executable(TwoLevelML_error_rate_2d TwoLevelML_error_rate_2d.cpp)

target_link_libraries(TwoLevelML_error_rate_2d PUBLIC
    error_dynamics
    decoder
    pybind11::embed
)