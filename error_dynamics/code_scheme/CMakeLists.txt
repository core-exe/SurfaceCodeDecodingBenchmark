add_library(error_dynamics_code_scheme STATIC
    code_scheme.hpp
    rectangular_scheme.hpp
    rectangular_scheme.cpp
)

target_link_libraries(error_dynamics_code_scheme PUBLIC
    error_dynamics_util
)

target_include_directories(error_dynamics_code_scheme PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
