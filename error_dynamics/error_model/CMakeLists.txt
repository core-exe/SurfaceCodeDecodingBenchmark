add_library(error_dynamics_error_model STATIC
    error_model_base.hpp
    iid_error.cpp
    iid_error.hpp
    error_model.hpp
)

target_link_libraries(error_dynamics_error_model PUBLIC
    error_dynamics_util
    error_dynamics_code_scheme
)

target_include_directories(error_dynamics_error_model PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
