# Building ModBrowser lib




set( SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameRoot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabAbout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabGames.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabGeneralSettings.cpp
    )




add_library( GameBrowser STATIC ${SRC_FILES} )
install( TARGETS GameBrowser DESTINATION lib )

target_include_directories( GameBrowser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries( GameBrowser PUBLIC
    Borealis
    ModBrowser
    switch::libnx
    -L/opt/devkitpro/portlibs/switch/lib
    -L/opt/devkitpro/libnx/lib
    ${ZLIB_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    -lglfw3 -lEGL -lglad -lglapi -ldrm_nouveau -lm -lnx
    )
