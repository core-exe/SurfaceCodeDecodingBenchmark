add_library(error_dynamics_util STATIC
    util.hpp
    constant.hpp
    exception.cpp
    exception.hpp
    display.hpp
)
set_target_properties(error_dynamics_util PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_include_directories(error_dynamics_util PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")