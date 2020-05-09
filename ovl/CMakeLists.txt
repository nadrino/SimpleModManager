#
# CMakeLists for the Tesla Overlay
#

# Replace this with the name of your application
set(OVL_NAME "SimpleModManagerOverlay")
set(OVL_APP "${OVL_NAME}")

# Meta information about the app
set(OVL_TITLE ${OVL_NAME})
set(OVL_AUTHOR "Nadrino")
set(OVL_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.jpg")

set(OVL_SOURCE_DIR ${PROJECT_SOURCE_DIR}/ovl/source)

file(GLOB OVL_SOURCE_FILES "${OVL_SOURCE_DIR}/*.cpp")
file(GLOB OVL_TESLA_FILES "${PROJECT_SOURCE_DIR}/ovl/libs/libtesla/include/*.hpp" "${PROJECT_SOURCE_DIR}/ovl/libs/libtesla/include/*.h")

include_directories("${PROJECT_SOURCE_DIR}/ovl/libs/libtesla/include/")
include(nx-utils)

cmake_info("Source Dir : ${OVL_SOURCE_DIR}")

if (cmake_VERBOSE)
    list(LENGTH OVL_SOURCE_FILES OVL_SOURCE_FILES_LENGTH)
    cmake_info("Loaded ${OVL_SOURCE_FILES_LENGTH} source file(s)")
endif ()

set(SMM_SOURCES ${SOURCE_FILES})
list(REMOVE_ITEM SMM_SOURCES ${PROJECT_SOURCE_DIR}/source/main.cpp)

add_executable(${OVL_APP}.elf
        #        ${SOURCE_DIR}/main.cpp
        ${OVL_SOURCE_FILES}
        ${OVL_TESLA_FILES}
        ${SMM_SOURCES}
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
        LINK_FLAGS "-specs=${LIBNX}/switch.specs -Wl,-no-as-needed -Wl,-Map,.map")

build_switch_ovl_binaries(${OVL_APP}.elf)




