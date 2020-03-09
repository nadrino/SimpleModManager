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

#find_package(SDL2 REQUIRED)
#message("SDL2_INCLUDE_DIRS = ${SDL2_INCLUDE_DIRS}")
#message("SDL2_LIBRARIES = ${SDL2_LIBRARIES}")


include_directories(${PROJECT_BINARY_DIR})
include_directories("${PORTLIBS}/include")
include_directories("${LIBNX}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${ZLIB_INCLUDE_DIRS}")
#include_directories("${SDL2_INCLUDE_DIRS}")

set(VERSION_MAJOR 1)
set(VERSION_MINOR 4)
set(VERSION_MICRO 1)
set(APP_VERSION
    "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}")

add_definitions( -DVERSION_APP=${APP_VERSION} )
add_definitions( -DVERSION_MAJOR_APP=${VERSION_MAJOR} )
add_definitions( -DVERSION_MINOR_APP=${VERSION_MINOR} )
add_definitions( -DVERSION_MICRO_APP=${VERSION_MICRO} )

# Replace this with the name of your application
set(APP_NAME "SimpleModManager")
set(HOMEBREW_APP "${APP_NAME}_${VERSION_MAJOR}_${VERSION_MINOR}_${VERSION_MICRO}")

# Meta information about the app
set(APP_TITLE ${APP_NAME})
set(APP_AUTHOR "Nadrino")
set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.jpg")

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

configure_file(version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h)
include_directories( ${CMAKE_BINARY_DIR}/generated/ )

#include(include/CMakeLists.txt)
include(source/CMakeLists.txt)

target_link_libraries(
        ${HOMEBREW_APP}.elf
        switch::libnx
        -L/opt/devkitpro/portlibs/switch/lib
        ${ZLIB_LIBRARIES}
#        -lSDL2 -march=armv8-a -fPIE -L/opt/devkitpro/libnx/lib -lEGL -lglapi -ldrm_nouveau -lnx
)
set_target_properties(${HOMEBREW_APP}.elf PROPERTIES
        LINKER_LANGUAGE CXX # Replace this with C if you have C source files
        LINK_FLAGS "-specs=${LIBNX}/switch.specs -Wl,-no-as-needed -Wl,-Map,.map")

build_switch_binaries(${HOMEBREW_APP}.elf)
