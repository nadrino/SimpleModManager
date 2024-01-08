#
# Building ModManagerCore lib
#


set( SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameBrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ModManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ModsPreseter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConfigHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Selector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Toolbox.cpp
    )


add_library( ModManagerCore STATIC ${SRC_FILES} )
install( TARGETS ModManagerCore DESTINATION lib )

target_include_directories( ModManagerCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries( ModManagerCore PUBLIC
    switch::libnx
    -L/opt/devkitpro/portlibs/switch/lib
    -L/opt/devkitpro/libnx/lib
    ${ZLIB_LIBRARIES}
    ${FREETYPE_LIBRARIES}
#    -lglfw3 -lEGL -lglad -lglapi -ldrm_nouveau -lm -lnx
    )
