add_executable(demo_coding_scheme demo_coding_scheme.cpp)
target_link_libraries(demo_coding_scheme PUBLIC error_dynamics)

add_executable(demo_error_model demo_error_model.cpp)
target_link_libraries(demo_error_model PUBLIC error_dynamics)

add_executable(demo_error_dynamics demo_error_dynamics.cpp)
target_link_libraries(demo_error_dynamics PUBLIC error_dynamics)

add_executable(demo_syndrome_graph demo_syndrome_graph.cpp)
target_link_libraries(demo_syndrome_graph PUBLIC error_dynamics decoder)

add_executable(demo_MWPM_decoder demo_MWPM_decoder.cpp)
target_link_libraries(demo_MWPM_decoder PUBLIC error_dynamics decoder)

add_executable(demo_ml_decoder demo_ml_decoder.cpp)
target_link_libraries(demo_ml_decoder PUBLIC error_dynamics decoder pybind11::embed)