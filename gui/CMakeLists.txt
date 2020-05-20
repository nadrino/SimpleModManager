#
# CMakeLists for the Gui App
#

# Replace this with the name of your application
set(GUI_NAME "SimpleModManagerGui")
set(GUI_APP "${GUI_NAME}")

# Meta information about the app
set(GUI_TITLE ${GUI_NAME})
set(GUI_AUTHOR "Nadrino")
set(GUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon_gui.jpg")

set(GUI_INC_DIR ${PROJECT_SOURCE_DIR}/gui/include)
include_directories(${GUI_INC_DIR})
include_directories(${GUI_INC_DIR}/draw)

set(GUI_SOURCE_DIR ${PROJECT_SOURCE_DIR}/gui/source)

file(GLOB GUI_SOURCE_FILES
        "${GUI_SOURCE_DIR}/*.cpp"
        "${GUI_SOURCE_DIR}/draw/*.cpp"
        )


include(nx-utils)

cmake_info("Source Dir : ${GUI_SOURCE_DIR}")

if (cmake_VERBOSE)
    list(LENGTH GUI_SOURCE_FILES GUI_SOURCE_FILES_LENGTH)
    cmake_info("Loaded ${GUI_SOURCE_FILES_LENGTH} source file(s)")
endif ()

#set(SMM_SOURCES ${SOURCE_FILES} source/Application.cc include/Application.h)
list(REMOVE_ITEM SMM_SOURCES ${PROJECT_SOURCE_DIR}/source/main.cpp)

add_executable(${GUI_APP}.elf
        #        ${SOURCE_DIR}/main.cpp
        ${GUI_SOURCE_FILES}
        ${SMM_SOURCES}
        )

target_link_libraries(
        ${GUI_APP}.elf
        switch::libnx
        -L/opt/devkitpro/portlibs/switch/lib
#        ${ZLIB_LIBRARIES}
#        -lSDL2 -march=armv8-a -fPIE -L/opt/devkitpro/libnx/lib -lEGL -lglapi -ldrm_nouveau -lnx
#        -lnx -lfreetype -lSDL2_ttf -lSDL2_gfx -lSDL2_image -lSDL2 -lEGL -lGLESv2 -lglapi -ldrm_nouveau -lpng -ljpeg -L/opt/devkitpro/portlibs/switch/lib -lSDL2 -march=armv8-a -fPIE -L/opt/devkitpro/libnx/lib -lEGL -lglapi -ldrm_nouveau -lnx -L/opt/devkitpro/portlibs/switch/lib -lfreetype -lbz2 -lpng16 -lz -lm -lnx -lz -lwebp
        -lSDL2  -march=armv8-a -fPIE -lEGL -lglapi -ldrm_nouveau -lSDL2_ttf -lfreetype
        -lpng -ljpeg -lnx
)

set_target_properties(${GUI_APP}.elf PROPERTIES
        LINKER_LANGUAGE CXX # Replace this with C if you have C source files
        LINK_FLAGS "-specs=${LIBNX}/switch.specs -Wl,-no-as-needed -Wl,-Map,.map")


build_switch_binaries(
        ${GUI_APP}.elf
        ${GUI_TITLE} ${GUI_AUTHOR} ${GUI_ICON} ${APP_VERSION}
)



