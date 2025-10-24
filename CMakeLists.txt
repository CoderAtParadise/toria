cmake_minimum_required(VERSION 4.1 FATAL_ERROR)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO "b80be207-778e-46ba-8080-b23bba22639e")
project(toria
        VERSION 0.1
        LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_CXX_MODULE_STD 1)

add_library(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

option(ASAN "Build this target with AddressSanitizer")

if (ASAN)
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PUBLIC /fsanitize=address)
    else ()
        target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
        target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
    endif ()
endif ()


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

option(BUILD_TESTS "Build Tests" OFF)
if(BUILD_TEST)
    add_subdirectory(tests)
endif ()
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
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        FILE_SET ${PROJECT_NAME}_crypto
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}"
        FILE_SET ${PROJECT_NAME}_util
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}"
        FILE_SET ${PROJECT_NAME}_uuid
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}"
)

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        CXX_MODULES_DIRECTORY
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/")

export(EXPORT ${PROJECT_NAME}-targets FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-targets.cmake" NAMESPACE toria::)
