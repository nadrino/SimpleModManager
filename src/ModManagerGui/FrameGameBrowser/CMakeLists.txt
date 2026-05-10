# Building ModBrowser lib




set( SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameRoot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabAbout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabGames.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ModsMtpServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabImportMod.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabGeneralSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpDataPacket.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpDebug.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpDeviceInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpEventPacket.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpObjectInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpPacket.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpProperty.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpRequestPacket.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpResponsePacket.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpServer.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpStorage.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpStorageInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpStringBuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/MtpUtils.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/USBMtpInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/USBSerialInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/log.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/nxlink.cpp
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/source/usb.c
    )




add_library( FrameGameBrowser STATIC ${SRC_FILES} )
install( TARGETS FrameGameBrowser DESTINATION lib )

target_include_directories( FrameGameBrowser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/ThirdParty/mtp-server-nx/include
    /opt/devkitpro/portlibs/switch/include
    )

target_link_libraries( FrameGameBrowser PUBLIC
    Borealis
    FrameModBrowser
    switch::libnx
    -L/opt/devkitpro/portlibs/switch/lib
    -L/opt/devkitpro/libnx/lib
    ${ZLIB_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    -lminizip -lz -lbz2 -lglfw3 -lEGL -lglad -lglapi -ldrm_nouveau -lm -lnx
    )
