# Qwertycoin: C++ client, tools and libraries.
# Copyright 2019 (c) The Qwertycoin Group.
# Licensed under the GNU General Public License, Version 3.
# See the file LICENSE from this package for details.

cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0074)
    # Allow find_package() to use <PackageName>_ROOT variables.
    # This policy was introduced in CMake version 3.12.
    cmake_policy(SET CMP0074 NEW)
endif()

if(POLICY CMP0087)
    # Install CODE|SCRIPT allow the use of generator expressions.
    cmake_policy(SET CMP0087 NEW)
endif()

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/cmake/find")

# Configure project build type.
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Supported build types" FORCE)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "CMake build type: ${CMAKE_BUILD_TYPE}")

# Configure Hunter (CMake driven cross-platform package manager for C/C++).
# WARNING: Must be included before project(...) function call!
set(HUNTER_CONFIGURATION_TYPES Release;Debug CACHE STRING "Hunter configuration types")
set(HUNTER_JOBS_NUMBER 4 CACHE STRING "Hunter jobs number")
include(hunter/HunterGate)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.198.tar.gz"
    SHA1 "f44b698691eedfe9d1ee6763a6fb9b4af7756c24"
    FILEPATH "${CMAKE_CURRENT_LIST_DIR}/cmake/hunter/HunterConfig.cmake"
)

# Read project version from git tag.
find_package(Git)
message(STATUS "Found Git: ${GIT_FOUND}")
if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --tags --abbrev=0 --match "v*"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE PROJECT_GIT_RESULT
        OUTPUT_VARIABLE PROJECT_GIT_TAG
        ERROR_VARIABLE PROJECT_GIT_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE)
    if(NOT PROJECT_GIT_RESULT EQUAL 0)
        set(PROJECT_GIT_TAG "0.0.0")
        message(WARNING "Failed to execute Git: ${PROJECT_GIT_ERROR}")
    else()
        string(REPLACE "v" "" PROJECT_GIT_TAG ${PROJECT_GIT_TAG})
        message(STATUS "Git tag: ${PROJECT_GIT_TAG}")
    endif()

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE PROJECT_GIT_RESULT
        OUTPUT_VARIABLE PROJECT_GIT_COMMIT_ID
        ERROR_VARIABLE PROJECT_GIT_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE)
    if(NOT PROJECT_GIT_RESULT EQUAL 0)
        set(PROJECT_GIT_COMMIT_ID "unknown")
        message(WARNING "Failed to execute Git: ${PROJECT_GIT_ERROR}")
    else()
        message(STATUS "Git commit ID: ${PROJECT_GIT_COMMIT_ID}")
    endif()
endif()

set(PROJECT_DISPLAY_NAME "Qwertycoin")
set(PROJECT_SHORT_NAME "qwcz")
set(PROJECT_NAME "qwertycoin-zero")
set(PROJECT_CURRENCY_TICKER "QWC")
set(PROJECT_DESCRIPTION "Safe. Simple. Seamless. Secure.")
set(PROJECT_VERSION "${PROJECT_GIT_TAG}")
set(PROJECT_COPYRIGHT "Copyright 2019 The Qwertycoin Group")
set(PROJECT_VENDOR_NAME "The Qwertycoin Group")
set(PROJECT_VENDOR_URL "https://qwertycoin.org")
set(PROJECT_VENDOR_DOMAIN_REVERSED "org.qwertycoin")
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES C CXX)

if(APPLE)
    # Compile src/Platform/OSX/System/asm.s file.
    enable_language(ASM)
endif()

include(project/CompilerSettings)
include(project/GlobalOptions)
include(project/GlobalVariables)

# External dependencies
set_directory_properties(PROPERTIES EP_BASE "${CMAKE_CURRENT_BINARY_DIR}/_ExternalProjects")
include(ExternalProject)
include(external/Boost)
include(external/qrencode)
include(external/Qt5)
include(external/Qwertycoin)
include(external/sparsehash)
include(external/codecov)
include(external/Threads)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src")

if(BUILD_WITH_PACKAGE)
    include(project/CPackConfig)
endif()
