#
# CMakeLists for the Gui App
#

# Replace this with the name of your application
set(TEST_NAME "SimpleModManagerTest")
set(TEST_APP "${TEST_NAME}")

cmake_info("Building SimpleModManagerTest")


set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)

# Meta information about the app
set(TEST_TITLE ${TEST_NAME})
set(TEST_AUTHOR "Nadrino")
set(TEST_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon_gui.jpg")
set(TEST_ROMFS "${TEST_DIR}/romfs")

set(BOREALIS_DIR ${TEST_DIR}/../gui/library/borealis/)
add_compile_definitions(BOREALIS_RESOURCES="romfs:/")

cmake_info("BOREALIS_DIR is ${BOREALIS_DIR}")

set(TEST_SOURCE_DIR ${TEST_DIR}/source)

file(GLOB TEST_SOURCE_FILES
        "${TEST_SOURCE_DIR}/*.cpp"
        "${BOREALIS_DIR}/library/lib/*.cpp"
        "${BOREALIS_DIR}/library/lib/extern/*/*.c"
        "${BOREALIS_DIR}/library/lib/extern/*/*/*.c"
        "${BOREALIS_DIR}/library/lib/*.cpp"
        "${BOREALIS_DIR}/library/lib/*.c"
        "${BOREALIS_DIR}/library/lib/extern/glad/*.c"
        "${BOREALIS_DIR}/library/lib/extern/nanovg/*.c"
        "${BOREALIS_DIR}/library/lib/extern/libretro-common/compat/*.c"
        "${BOREALIS_DIR}/library/lib/extern/libretro-common/encodings/*.c"
        "${BOREALIS_DIR}/library/lib/extern/libretro-common/features/*.c"
        )

cmake_info("Sources : ${TEST_SOURCE_FILES}")


#set(TEST_INC_DIR ${TEST_DIR}/include)
set(BOREALIS_INC_DIR ${BOREALIS_DIR}/library/include)
#set(PLUTONIUM_INC_DIR ${PLUTONIUM_DIR}/Include)

include(nx-utils)

cmake_info("Source Dir : ${TEST_SOURCE_DIR}")

if (cmake_VERBOSE)
    list(LENGTH TEST_SOURCE_FILES TEST_SOURCE_FILES_LENGTH)
    cmake_info("Loaded ${TEST_SOURCE_FILES_LENGTH} source file(s)")
endif ()

#set(SMM_SOURCES ${SOURCE_FILES} source/Application.cc include/Application.h)
list(REMOVE_ITEM SMM_SOURCES ${PROJECT_SOURCE_DIR}/source/main.cpp)

add_executable(${TEST_APP}.elf
#        ${SOURCE_DIR}/main.cpp
        ${TEST_SOURCE_FILES}
#        ${SMM_SOURCES}
        )

target_include_directories(${TEST_APP}.elf PUBLIC
        ${TEST_INC_DIR}
        ${BOREALIS_INC_DIR}
        ${BOREALIS_INC_DIR}/borealis
        ${BOREALIS_INC_DIR}/borealis/extern/glad
        ${BOREALIS_INC_DIR}/borealis/extern/nanovg
        ${BOREALIS_INC_DIR}/borealis/extern/libretro-common
        #        ${TEST_INC_DIR}/draw
)

target_link_libraries(
        ${TEST_APP}.elf
#        switch::libnx
#        -L/opt/devkitpro/portlibs/switch/lib
#        -L/opt/devkitpro/libnx/lib
#        ${ZLIB_LIBRARIES}
#        ${FREETYPE_LIBRARIES}
#        -lglfw3 -lEGL -lglapi -ldrm_nouveau -lm -lnx
        -L${DEVKITPRO}/portlibs/switch/lib -L${DEVKITPRO}/libnx/lib -lglfw3 -lEGL -lglapi -ldrm_nouveau -lm -lnx -lm
#        -lSDL2 -march=armv8-a -fPIE -lEGL -lglapi -ldrm_nouveau -lnx
#        -lnx -lfreetype -lSDL2_ttf -lSDL2_gfx -lSDL2_image -lSDL2 -lEGL -lGLESv2 -lglapi -ldrm_nouveau -lpng -ljpeg -L/opt/devkitpro/portlibs/switch/lib -lSDL2 -march=armv8-a -fPIE -L/opt/devkitpro/libnx/lib -lEGL -lglapi -ldrm_nouveau -lnx -L/opt/devkitpro/portlibs/switch/lib -lfreetype -lbz2 -lpng16 -lz -lm -lnx -lz -lwebp
#        -lSDL2  -march=armv8-a -fPIE -lEGL -lglapi -ldrm_nouveau -lSDL2_ttf -lfreetype
#        -lpng -ljpeg -lnx
#        -lfreetype -lSDL2_mixer -lopusfile -lopus -lmodplug -lmpg123 -lvorbisidec -logg -lSDL2_ttf -lSDL2_gfx -lSDL2_image -lSDL2 -lEGL -lGLESv2 -lglapi -ldrm_nouveau -lwebp -lpng -ljpeg
)

set_target_properties(${TEST_APP}.elf PROPERTIES
        LINKER_LANGUAGE CXX # Replace this with C if you have C source files
        LINK_FLAGS "-specs=${LIBNX}/switch.specs -g -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE -Wl,-Map,sys-clk-manager.map"
        )


#set(CMAKE_BUILD_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TEST_APP}.elf.dir)
#set(BUILD_ROMFS ${CMAKE_BUILD_DIRECTORY}/Resources)
#cmake_info("Resources will be gathered in: ${BUILD_ROMFS}")
#add_custom_command(
#        TARGET ${TEST_APP}.elf
#        PRE_BUILD
#        COMMAND ${CMAKE_COMMAND} -E echo "Gathering resources..."
#        COMMAND ${CMAKE_COMMAND} -E remove_directory ${BUILD_ROMFS}
#        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_ROMFS}
#        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_ROMFS}/borealis
#        COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEST_ROMFS}/. ${BUILD_ROMFS}/.
#        COMMAND ${CMAKE_COMMAND} -E copy_directory ${BOREALIS_DIR}/resources/. ${BUILD_ROMFS}/borealis/.
#        COMMAND ${CMAKE_COMMAND} -E echo "Resources have been gathered."
#)

build_switch_binaries(
        ${TEST_APP}.elf
        ${TEST_TITLE} ${TEST_AUTHOR} ${TEST_ICON} ${APP_VERSION} ${BOREALIS_DIR}/resources/
)

