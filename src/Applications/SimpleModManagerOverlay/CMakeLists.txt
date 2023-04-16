#
# CMakeLists for the Tesla Overlay
#

# Replace this with the name of your application
set( OVL_NAME "SimpleModManagerOverlay" )
set( OVL_APP "${OVL_NAME}" )

# Meta information about the app
set( OVL_TITLE "SimpleModManager" )
set( OVL_AUTHOR "Nadrino" )
set( OVL_ICON ${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.jpg )

# sources
set( SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/SimpleModManagerOverlay.cpp )


add_executable( ${OVL_APP}.elf ${SRC_FILES} )

target_link_libraries( ${OVL_APP}.elf
    ModManagerOverlay
    switch::libnx
    -L/opt/devkitpro/portlibs/switch/lib
    ${ZLIB_LIBRARIES}
    )

set_target_properties( ${OVL_APP}.elf PROPERTIES
    LINKER_LANGUAGE CXX # Replace this with C if you have C source files
    LINK_FLAGS "-specs=${LIBNX}/switch.specs -Wl,-no-as-needed -Wl,-Map,.map"
    )

build_switch_ovl_binaries( ${OVL_APP}.elf ${OVL_TITLE} ${OVL_AUTHOR} ${OVL_ICON} ${APP_VERSION} )

