add_subdirectory(code_scheme)
add_subdirectory(error_model)
add_subdirectory(util)

link_directories(code_scheme error_model util)
include_directories(code_scheme error_model util)

add_library(error_dynamics STATIC error_dynamics.hpp)

target_link_libraries(error_dynamics STATIC
    error_dynamics_util
    error_dynamics_error_model
    error_dynamics_coding_scheme
)