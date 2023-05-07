#
# Building CoreExtension app
#

set( SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GuiModManager.cpp
    )

add_library( CoreExtension STATIC ${SRC_FILES} )
install( TARGETS CoreExtension DESTINATION lib )

target_include_directories( CoreExtension PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries( CoreExtension PUBLIC
    Borealis
    ModManagerCore
    switch::libnx
    -L/opt/devkitpro/portlibs/switch/lib
    -L/opt/devkitpro/libnx/lib
    ${ZLIB_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    -lglfw3 -lEGL -lglad -lglapi -ldrm_nouveau -lm -lnx
    )
