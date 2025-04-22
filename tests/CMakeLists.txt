add_executable(toria_test main.cxx)
target_compile_features(toria_test PRIVATE cxx_std_23)
target_link_libraries(toria_test toria)
