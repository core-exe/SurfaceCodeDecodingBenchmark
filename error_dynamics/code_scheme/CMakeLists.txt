add_library(error_dynamics_code_scheme STATIC
    code_scheme.hpp
    planar_scheme.hpp
    planar_scheme.cpp
)

target_link_libraries(error_dynamics_code_scheme PUBLIC
    error_dynamics_util
)
set_target_properties(error_dynamics_code_scheme PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_include_directories(error_dynamics_code_scheme PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
