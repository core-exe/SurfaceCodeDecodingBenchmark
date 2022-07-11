add_library(error_dynamics_error_model STATIC
    error_model_base.cpp
)

target_include_directories(error_dynamics_error_model PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
