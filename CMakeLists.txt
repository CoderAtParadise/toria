cmake_minimum_required (VERSION 3.28)
project (toria)


# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

add_subdirectory(std)
# Add source to this project's executable.
add_library (toria)
target_compile_features(toria PRIVATE cxx_std_23)
target_sources(toria PUBLIC FILE_SET CXX_MODULES FILES toria.cxx  uuid.cxx generators.cxx std.cxx md5.cxx sha1.cxx)
target_link_libraries(toria std)

add_executable(hello main.cxx)
target_compile_features(hello PRIVATE cxx_std_23)
target_link_libraries(hello toria std)

# TODO: Add tests and install targets if needed.
