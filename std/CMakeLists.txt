cmake_minimum_required(VERSION 3.28)

project(std)

add_library(${PROJECT_NAME} STATIC)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

# hacky :/

string(REGEX REPLACE "\/bin\/Hostx(64|86)\/x(64|86)\/cl\.exe" "" MSVC_ROOT ${CMAKE_CXX_COMPILER})

# std.ixx
configure_file("${MSVC_ROOT}/modules/std.ixx" "${PROJECT_BINARY_DIR}/std.ixx" COPYONLY)
string(JOIN " " STD_REF_PARAM /reference "${PROJECT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/std.ifc")
set(STD_REFERENCE ${STD_REF_PARAM} CACHE INTERNAL "std.ifc compiler parameter")

# std.compat.ixx
configure_file("${MSVC_ROOT}/modules/std.compat.ixx" "${PROJECT_BINARY_DIR}/std.compat.ixx" COPYONLY)
set(STD_COMPAT_REFERENCE /reference "${PROJECT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/std.compat.ifc" CACHE INTERNAL "std.compat.ifc compiler parameter")

target_sources(${PROJECT_NAME}
		PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES BASE_DIRS ${PROJECT_BINARY_DIR}
		FILES ${PROJECT_BINARY_DIR}/std.ixx ${PROJECT_BINARY_DIR}/std.compat.ixx
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
