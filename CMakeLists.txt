cmake_minimum_required(VERSION 4.3 FATAL_ERROR)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
set(CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO "ababa1b5-7099-495f-a9cd-e22d38f274f2")
set(CMAKE_CXX_MODULE_STD 1)
project(toria
        VERSION 0.1.0
        LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(GNUInstallDirs)
include(CTest)

string(TOUPPER ${PROJECT_NAME} OPTION_PREFIX)

option(TORIA_BUILD_WITH_ASAN "Build with AddressSanitizer" OFF)
cmake_dependent_option(TORIA_BUILD_TESTS "Build toria Tests" ON "BUILD_TESTING;PROJECT_IS_TOP_LEVEL" OFF)
option(TORIA_INSTALL_USING_CPS "Install using cps" OFF)

add_library(${PROJECT_NAME} STATIC)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
add_library(toria STATIC)
target_compile_features(toria PUBLIC cxx_std_23)

if (TORIA_BUILD_WITH_ASAN)
    if (MSVC)
        target_compile_options(toria PUBLIC /fsanitize=address)
    else ()
        target_compile_options(toria PUBLIC -fsanitize=address)
        target_link_options(toria PUBLIC -fsanitize=address)
    endif ()
endif ()

#[[==CRYPTO==]]
target_sources(
        toria PUBLIC
        FILE_SET toria_crypto
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
        toria PUBLIC
        FILE_SET toria_system
        TYPE CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/system"
        FILES
        src/system/process.cppm
        src/system/std.cppm
        src/system/system.cppm
        src/system/system_info.cppm
        src/system/thread.cppm
)
if (WIN32)
    target_sources(
            toria PRIVATE
            src/system/windows.cpp
    )
endif ()

#[[==SEMVER==]]
target_sources(
        toria PUBLIC
        FILE_SET toria_semver
        TYPE CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/semver"
        FILES
        src/semver/semver.cppm
        src/semver/impl.cppm
        src/semver/range.cppm
        src/semver/std.cppm
)

#[[==UTIL==]]
target_sources(
        toria PUBLIC
        FILE_SET toria_util
        TYPE CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/util"
        FILES
        src/util/basic_fixed_string.cppm
        src/util/byte_utils.cppm
        src/util/instance_of.cppm
        src/util/std.cppm
        src/util/string_utils.cppm
        src/util/util.cppm
)

if (WIN32)
    target_sources(
            toria PRIVATE
            src/util/windows.cpp
    )
endif ()

#[[==UUID==]]
target_sources(
        toria PUBLIC
        FILE_SET toria_uuid
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
        FILE_SET ${PROJECT_NAME}_system
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}/system"
        FILE_SET ${PROJECT_NAME}_semver
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}/semver"
        FILE_SET ${PROJECT_NAME}_util
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}/util"
        FILE_SET ${PROJECT_NAME}_uuid
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}/uuid"
install(TARGETS toria
        EXPORT toria-targets
        FILE_SET toria_coro
        DESTINATION "${CMAKE_INSTALL_DATADIR}/toria/cxx/crypto"
        FILE_SET toria_memory
        DESTINATION "${CMAKE_INSTALL_DATADIR}/toria/cxx/system"
        FILE_SET toria_semver
        DESTINATION "${CMAKE_INSTALL_DATADIR}/toria/cxx/semver"
        FILE_SET toria_util
        DESTINATION "${CMAKE_INSTALL_DATADIR}/toria/cxx/util"
        FILE_SET toria_uuid
        DESTINATION "${CMAKE_INSTALL_DATADIR}/toria/cxx/uuid"
        CXX_MODULES_BMI DESTINATION "${CMAKE_INSTALL_LIBDIR}/bmi"
)

if (TORIA_INSTALL_USING_CPS)
    install(
            PACKAGE_INFO toria
            EXPORT toria-targets
            CXX_MODULES_DIRECTORY "modules"
    )
endif ()

install(EXPORT toria-targets
        FILE "toria-config.cmake"
        NAMESPACE toria::
        CXX_MODULES_DIRECTORY DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/toria"
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/toria-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/toria-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/toria")

if (TORIA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()
