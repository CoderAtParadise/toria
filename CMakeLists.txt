cmake_minimum_required(VERSION 4.1 FATAL_ERROR)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
project(toria
        VERSION 0.1.0
        LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_CXX_MODULE_STD 1)

string(TOUPPER ${PROJECT_NAME} OPTION_PREFIX)

option(${OPTION_PREFIX}_BUILD_WITH_ASAN "Build with AddressSanitizer" OFF)
option(${OPTION_PREFIX}_BUILD_TESTS "Build Tests" OFF)

add_library(${PROJECT_NAME} STATIC)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

if (${OPTION_PREFIX}_BUILD_WITH_ASAN)
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

#[[==SYSTEM==]]
target_sources(
        ${PROJECT_NAME} PUBLIC
        FILE_SET ${PROJECT_NAME}_system
        TYPE CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/system"
        FILES
        src/system/process.cppm
        src/system/std.cppm
        src/system/system.cppm
        src/system/thread.cppm
)
if (WIN32)
    target_sources(
            ${PROJECT_NAME} PRIVATE
            src/system/windows.cpp
    )
endif ()

#[[==UTIL==]]
target_sources(
        ${PROJECT_NAME} PUBLIC
        FILE_SET ${PROJECT_NAME}_util
        TYPE CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/util"
        FILES
        src/util/basic_fixed_string.cppm
        src/util/byte_utils.cppm
        src/util/std.cppm
        src/util/string_utils.cppm
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

#[[==INSTALL EXPORT TARGETS==]]
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        FILE_SET ${PROJECT_NAME}_crypto
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}/crypto"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}/system"
        FILE_SET ${PROJECT_NAME}_semver
        FILE_SET ${PROJECT_NAME}_util
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}/util"
        FILE_SET ${PROJECT_NAME}_uuid
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}/uuid"
)

install(EXPORT ${PROJECT_NAME}-targets
        FILE "${PROJECT_NAME}-config.cmake"
        NAMESPACE ${PROJECT_NAME}::
        CXX_MODULES_DIRECTORY
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

if (${OPTION_PREFIX}_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()
