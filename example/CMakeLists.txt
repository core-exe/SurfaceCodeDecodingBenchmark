link_directories(../error_dynamics)
include_directories(../error_dynamics)

add_executable(demo_coding_scheme demo_coding_scheme.cpp)
target_link_libraries(demo_coding_scheme PRIVATE error_dynamics)