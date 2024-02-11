cmake_minimum_required (VERSION 3.28)
project (toria)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


# Add source to this project's executable.
add_library (toria)

target_compile_features(toria PRIVATE cxx_std_23)
target_sources(toria PUBLIC FILE_SET CXX_MODULES FILES toria.cxx  impl.cxx generators.cxx std.cxx)

add_executable(hello main.cxx)
target_compile_features(hello PRIVATE cxx_std_23)
target_link_libraries(hello toria)

install(TARGETS toria CXX_MODULES_BMI DESTINATION "${CMAKE_INSTALL_LIBDIR}/lib/cxx/bmi")
# TODO: Add tests and install targets if needed.
