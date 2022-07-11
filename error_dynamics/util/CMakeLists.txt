add_library(error_dynamics_util STATIC
    util.hpp
    constant.hpp
    exception.cpp
    exception.hpp
    display.hpp
)

target_include_directories(error_dynamics_util PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")