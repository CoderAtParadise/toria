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

option(ASAN "Build this target with AddressSanitizer" ON)

if(ASAN)
 if(MSVC)
  target_compile_options(${PROJECT_NAME} PUBLIC /fsanitize=address)
 else()
  target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
  target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
 endif()
endif()


#[[==CRYPTO==]]
target_sources(
    ${PROJECT_NAME} PUBLIC
    FILE_SET ${PROJECT_NAME}_crypto
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

#[[==UTIL==]]
 target_sources(
     ${PROJECT_NAME} PUBLIC
     FILE_SET ${PROJECT_NAME}_util
     TYPE CXX_MODULES
     BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/util"
     FILES
        src/util/byte_utils.cppm
        src/util/util.cppm
  )

#[[==UUID==]]

target_sources(
    ${PROJECT_NAME} PUBLIC
    FILE_SET ${PROJECT_NAME}_uuid
    TYPE CXX_MODULES
    BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/uuid"
    FILES
        src/uuid/generators.cppm
        src/uuid/impl.cppm     
        src/uuid/std.cppm
        src/uuid/uuid.cppm
 )

target_include_directories(${PROJECT_NAME} PUBLIC src/)

#[[==INSTALL EXPORT TARGETS==]]
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    FILE_SET ${PROJECT_NAME}_crypto
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}"
    FILE_SET ${PROJECT_NAME}_util
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}"
    FILE_SET ${PROJECT_NAME}_uuid
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}"
)


add_subdirectory(tests)
