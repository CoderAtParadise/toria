add_executable(${PROJECT_NAME}_test main.cxx
)
target_compile_features(${PROJECT_NAME}_test PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})
