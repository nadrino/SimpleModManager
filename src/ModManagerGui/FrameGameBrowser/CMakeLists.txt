# Building ModBrowser lib




set( SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameRoot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabAbout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabGames.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabGeneralSettings.cpp
    )




add_library( FrameGameBrowser STATIC ${SRC_FILES} )
install( TARGETS FrameGameBrowser DESTINATION lib )

target_include_directories( FrameGameBrowser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries( FrameGameBrowser PUBLIC
    Borealis
    FrameModBrowser
    switch::libnx
    -L/opt/devkitpro/portlibs/switch/lib
    -L/opt/devkitpro/libnx/lib
    ${ZLIB_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    -lglfw3 -lEGL -lglad -lglapi -ldrm_nouveau -lm -lnx
    )
