#
# Building Overlay lib
#

#set( SRC_FILES
##    ${CMAKE_CURRENT_SOURCE_DIR}/src/ChangeConfigPresetGui.cpp
##    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameBrowserGui.cpp
##    ${CMAKE_CURRENT_SOURCE_DIR}/src/ModBrowserGui.cpp
#    )
#
#
#
#add_library( ModManagerOverlay STATIC ${SRC_FILES} )
#target_include_directories( ModManagerOverlay PUBLIC ${SUBMODULES_DIR}/libtesla/include )
#install( TARGETS ModManagerOverlay DESTINATION lib )
#
#target_include_directories( ModManagerOverlay PUBLIC
#    ${CMAKE_CURRENT_SOURCE_DIR}/include
#    )
#
#target_link_libraries( ModManagerOverlay PUBLIC
#    ModManagerCore
#    switch::libnx
#    -L/opt/devkitpro/portlibs/switch/lib
#    -L/opt/devkitpro/libnx/lib
#    ${ZLIB_LIBRARIES}
#    ${FREETYPE_LIBRARIES}
#    #    -lglfw3 -lEGL -lglad -lglapi -ldrm_nouveau -lm -lnx
#    )
