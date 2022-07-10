add_library(error_dynamics_coding_scheme
    code_scheme.hpp
    rectangular_scheme.cpp
    rectangular_scheme.hpp
)

target_include_directories(error_dynamics_coding_scheme PUBLIC ../util)
target_link_libraries(error_dynamics_coding_scheme PUBLIC ../util)