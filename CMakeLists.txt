# Copyright 2019 SwitchPy Team. All rights reserved.
# Licensed under the MIT license.
# Refer to the LICENSE file included.
#
# libnx CMake template for Nintendo Switch homebrew development.

cmake_minimum_required(VERSION 3.1)

project(SimpleModManager)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(options)
include(utils)

find_package(LIBNX REQUIRED)
if (NOT LIBNX_FOUND)
    cmake_panic("Unable to detect libnx on this system.")
endif ()


find_package(ZLIB REQUIRED)
if (${ZLIB_FOUND})
    message("ZLIB found : ${ZLIB_VERSION_STRING}")
    message("ZLIB_INCLUDE_DIRS = ${ZLIB_INCLUDE_DIRS}")
    message("ZLIB_LIBRARIES = ${ZLIB_LIBRARIES}")
else()
    message(FATAL_ERROR "ZLIB has not been found.")
endif ()

find_package(Freetype REQUIRED)
if (${FREETYPE_FOUND})
    message("Freetype found : ${FREETYPE_VERSION_STRING}")
    message("FREETYPE_INCLUDE_DIRS = ${FREETYPE_INCLUDE_DIRS}")
    message("FREETYPE_LIBRARIES = ${FREETYPE_LIBRARIES}")
else()
    message(FATAL_ERROR "FREETYPE has not been found.")
endif ()


find_package(BZip2 REQUIRED)
if (${BZIP2_FOUND})
    message("BZIP2 found : ${BZIP2_VERSION_STRING}")
    message("BZIP2_INCLUDE_DIRS = ${BZIP2_INCLUDE_DIRS}")
    message("BZIP2_LIBRARIES = ${BZIP2_LIBRARIES}")
else()
    message(FATAL_ERROR "BZIP2 has not been found.")
endif ()

#find_package(SDL2 REQUIRED)
#message("SDL2_INCLUDE_DIRS = ${SDL2_INCLUDE_DIRS}")
#message("SDL2_LIBRARIES = ${SDL2_LIBRARIES}")


include_directories(${PROJECT_BINARY_DIR})
include_directories("${PORTLIBS}/include")
include_directories("${LIBNX}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${ZLIB_INCLUDE_DIRS}")
include_directories("${FREETYPE_INCLUDE_DIRS}")
#include_directories("${SDL2_INCLUDE_DIRS}")

set(VERSION_MAJOR 2)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(APP_VERSION
    "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}")

add_definitions( -DVERSION_APP=${APP_VERSION} )
add_definitions( -DVERSION_MAJOR_APP=${VERSION_MAJOR} )
add_definitions( -DVERSION_MINOR_APP=${VERSION_MINOR} )
add_definitions( -DVERSION_MICRO_APP=${VERSION_MICRO} )

if (NOT DEFINED CMAKE_BUILD_TYPE_INIT)
    set(CMAKE_BUILD_TYPE_INIT Release)
endif ()

set(CMAKE_C_STANDARD 99)
if (USE_CPP_17)
    set(CMAKE_CXX_STANDARD 17)
else ()
    set(CMAKE_CXX_STANDARD 11)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    include(clang-utils)
endif ()

include(nx-utils)

cmake_info("Building ${APP_TITLE} version ${APP_VERSION}.")


# SimpleModManager Core
set(SMM_CORE_DIR ${PROJECT_SOURCE_DIR}/core)
set(SMM_CORE_SOURCE_DIR ${SMM_CORE_DIR}/source)
set(SMM_CORE_INCLUDE_DIR ${SMM_CORE_DIR}/include)

include_directories(${SMM_CORE_INCLUDE_DIR})

# Submodules
set(SUBMODULES_DIR ${PROJECT_SOURCE_DIR}/submodules)


configure_file(version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h)
include_directories( ${CMAKE_BINARY_DIR}/generated/ )

include(core/CMakeLists.txt)
include(gui/CMakeLists.txt)
include(ovl/CMakeLists.txt)
include(external/sys-clk/manager/CMakeLists.txt)
