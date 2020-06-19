#
# CMakeLists for the Tesla Overlay
#

# Replace this with the name of your application
set(OVL_NAME "SimpleModManagerOverlay")
set(OVL_APP "${OVL_NAME}")

# Meta information about the app
set(OVL_TITLE "SimpleModManager")
set(OVL_AUTHOR "Nadrino")
set(OVL_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.jpg")

set(OVL_DIR ${PROJECT_SOURCE_DIR}/ovl)
set(OVL_LIBTESLA_DIR ${OVL_DIR}/libs/libtesla)
set(OVL_SOURCE_DIR ${OVL_DIR}/source)
set(OVL_INCLUDE_DIR ${OVL_DIR}/include)

# sources
file(GLOB OVL_SOURCE_FILES "${OVL_SOURCE_DIR}/*.cpp")
file(GLOB OVL_TESLA_FILES "${OVL_LIBTESLA_DIR}/include/*.hpp" "${OVL_LIBTESLA_DIR}/include/*.h")

# include
#include_directories("${OVL_INCLUDE_DIR}")
#include_directories("${OVL_LIBTESLA_DIR}/include")
include(nx-utils)

cmake_info("Source Dir : ${OVL_SOURCE_DIR}")

if (cmake_VERBOSE)
    list(LENGTH OVL_SOURCE_FILES OVL_SOURCE_FILES_LENGTH)
    cmake_info("Loaded ${OVL_SOURCE_FILES_LENGTH} source file(s)")
endif ()

set(SMM_SOURCES ${SOURCE_FILES})
list(REMOVE_ITEM SMM_SOURCES ${PROJECT_SOURCE_DIR}/source/main.cpp)

build_switch_ovl_binaries(${OVL_APP}.elf ${OVL_TITLE} ${OVL_AUTHOR} ${OVL_ICON} ${APP_VERSION})

add_executable(${OVL_APP}.elf
        #        ${SOURCE_DIR}/main.cpp
        ${OVL_SOURCE_FILES}
        ${OVL_TESLA_FILES}
        ${SMM_SOURCES}
        )

target_include_directories(
        ${OVL_APP}.elf
        PRIVATE
        ${OVL_INCLUDE_DIR}
        ${OVL_LIBTESLA_DIR}/include
)

target_link_libraries(
        ${OVL_APP}.elf
        switch::libnx
        -L/opt/devkitpro/portlibs/switch/lib
        ${ZLIB_LIBRARIES}
        #        -lSDL2 -march=armv8-a -fPIE -L/opt/devkitpro/libnx/lib -lEGL -lglapi -ldrm_nouveau -lnx
)

set_target_properties(${OVL_APP}.elf PROPERTIES
        LINKER_LANGUAGE CXX # Replace this with C if you have C source files
        LINK_FLAGS "-specs=${LIBNX}/switch.specs -Wl,-no-as-needed -Wl,-Map,.map"
        )





