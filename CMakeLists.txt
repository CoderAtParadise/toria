cmake_minimum_required(VERSION 3.30 FATAL_ERROR)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
project (toria
    VERSION 0.1
    LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_CXX_MODULE_STD 1)

add_library(${PROJECT_NAME})


#[[==UTIL==]]
 target_sources(
     ${PROJECT_NAME} PUBLIC
     FILE_SET ${PROJECT_NAME}_UTILS
     TYPE CXX_MODULES
     BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/util"
     FILES
        src/util/tracing_exception.cppm
        src/util/byte_utils.cppm
        src/util/util.cppm
 )

#[[==UUID==]]

target_sources(
    ${PROJECT_NAME} PUBLIC
    FILE_SET ${PROJECT_NAME}_UUID
    TYPE CXX_MODULES
    BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/uuid"
    FILES
        src/uuid/generators.cppm
        src/uuid/impl.cppm     
        src/uuid/std.cppm
        src/uuid/uuid.cppm
 )

#[[==CRYPTO==]]

target_sources(
    ${PROJECT_NAME} PUBLIC
    FILE_SET ${PROJECT_NAME}_CRYPTO
    TYPE CXX_MODULES
    BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/crypto"
    FILES
        src/crypto/common.cppm
        src/crypto/crypto.cppm
        src/crypto/md5.cppm
        src/crypto/hash.cppm
        src/crypto/sha1.cppm
        src/crypto/std.cppm
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    FILE_SET ${PROJECT_NAME}_UTILS
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}"
    FILE_SET ${PROJECT_NAME}_UUID
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}"
    FILE_SET ${PROJECT_NAME}_CRYPTO
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}"
)

#[[==INSTALL EXPORT TARGETS==]]

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

#[[==INSTALL EXPORT TARGETS==]]

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (NOT EXISTS "${PROJECT_BINARY_DIR}/${PRJECT_NAME}-config.cmake.in")
    file(WRITE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in [[
        @PACKAGE_INIT@
        include("${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-targets.cmake")
    ]])
endif()

configure_package_config_file(
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(
    EXPORT ${PROJECT_NAME}-targets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    NAMESPACE ${PROJECT_NAME}::)
install(
    FILES
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
add_subdirectory(tests)
