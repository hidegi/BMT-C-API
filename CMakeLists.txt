cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(bmt VERSION 2.0.2 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
include(utils)

if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()
if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()
if(POLICY CMP0068)
    cmake_policy(SET CMP0068 NEW)
endif()

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(BMT_TOP_LEVEL ON)
else()
    set(BMT_TOP_LEVEL OFF)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type: Debug|Release|RelWithDebInfo|MinSizeRel" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#-----------------------------------------------------------------------------
# Platform detection for platform.h generation
#-----------------------------------------------------------------------------
set(BMT_HAVE_BST_MAJOR_INCLINED 1)
set(BMT_STATIC_BUILD 1)
if(WIN32)
    set(BMT_PLATFORM_WINDOWS 1)
elseif(APPLE)
    set(BMT_PLATFORM_MACOS 1)
elseif(UNIX)
    set(BMT_PLATFORM_LINUX 1)
endif()

configure_file(
    "${PROJECT_SOURCE_DIR}/include/platform.h.in"
    "${PROJECT_BINARY_DIR}/include/platform.h"
)

#-----------------------------------------------------------------------------
# ZLIB — system install preferred, source fallback
#-----------------------------------------------------------------------------
include(FetchContent)
find_package(ZLIB QUIET)
if(NOT ZLIB_FOUND)
    message(STATUS "ZLIB not found on system — fetching from source")
    FetchContent_Declare(
        zlib_src
        URL      https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz
        URL_HASH SHA256=9a93b2b7dfdac77ceba5a558a580e74667dd6fede4585b91eefb60f03b72df23
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(zlib_src)
    if(TARGET zlibstatic AND NOT TARGET ZLIB::ZLIB)
        add_library(ZLIB::ZLIB ALIAS zlibstatic)
    endif()
endif()

#-----------------------------------------------------------------------------
# BMT — C99 static library (C-API only)
#-----------------------------------------------------------------------------
add_library(bmt STATIC
    src/bmt.c
    src/buffer.c
    src/treeops.c
    src/serialization.c
)

target_include_directories(bmt
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

target_link_libraries(bmt PRIVATE ZLIB::ZLIB)

set_target_properties(bmt PROPERTIES
    C_STANDARD          99
    C_STANDARD_REQUIRED ON
    VERSION             ${bmt_VERSION}
    SOVERSION           ${bmt_VERSION_MAJOR}
)

#-----------------------------------------------------------------------------
# Format targets (top-level only)
#-----------------------------------------------------------------------------
if(BMT_TOP_LEVEL)
    bmt_add_format_targets()
endif()

#-----------------------------------------------------------------------------
# Tests — off by default when used as a subdirectory / FetchContent dep
#-----------------------------------------------------------------------------
option(BMT_BUILD_TESTS "Build the BMT test suite" ${BMT_TOP_LEVEL})
if(BMT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
