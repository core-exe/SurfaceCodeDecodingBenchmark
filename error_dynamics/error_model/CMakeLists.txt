add_library(error_dynamics_error_model
    error_model_base.cpp
)

target_include_directories(error_dynamics_error_model PUBLIC "../util")
target_link_libraries(error_dynamics_error_model PUBLIC ../util)