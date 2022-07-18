add_subdirectory(util)
add_subdirectory(code_scheme)
add_subdirectory(error_model)

add_library(error_dynamics STATIC
    error_dynamics.hpp
    rectangular_surface_code.cpp
    rectangular_surface_code.hpp
)

target_link_libraries(error_dynamics PUBLIC
    error_dynamics_util
    error_dynamics_error_model
    error_dynamics_code_scheme
)

target_include_directories(error_dynamics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
