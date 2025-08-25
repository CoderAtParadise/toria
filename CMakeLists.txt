cmake_minimum_required(VERSION 4.1 FATAL_ERROR)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
project (toria
    VERSION 0.1
    LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_CXX_MODULE_STD 1)

add_library(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)


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

target_include_directories(${PROJECT_NAME} PUBLIC src/)

#[[==INSTALL EXPORT TARGETS==]]
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


add_subdirectory(tests)
