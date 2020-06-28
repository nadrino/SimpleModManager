#
# CMakeLists for the Gui App
#

# Replace this with the name of your application
set(GUI_NAME "SimpleModManagerGui")
set(GUI_APP "${GUI_NAME}")


set(GUI_DIR ${PROJECT_SOURCE_DIR}/gui)

# Meta information about the app
set(GUI_TITLE ${GUI_NAME})
set(GUI_AUTHOR "Nadrino")
set(GUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon_gui.jpg")
set(GUI_ROMFS "${GUI_DIR}/romfs")

set(BOREALIS_DIR ${SUBMODULES_DIR}/borealis)
add_compile_definitions(BOREALIS_RESOURCES=\"romfs:/borealis/\")

cmake_info("BOREALIS_DIR is ${BOREALIS_DIR}")

set(GUI_SOURCE_DIR ${GUI_DIR}/source)

file(GLOB GUI_SOURCE_FILES
        "${GUI_SOURCE_DIR}/*.cpp"
        "${GUI_SOURCE_DIR}/game_browser/*.cpp"
        "${GUI_SOURCE_DIR}/mod_browser/*.cpp"
        "${GUI_SOURCE_DIR}/core_extension/*.cpp"
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
#        "${SUBMODULES_DIR}/yaml-cpp/src/*.cpp"
        )


set(GUI_INC_DIR ${GUI_DIR}/include)
set(BOREALIS_INC_DIR ${BOREALIS_DIR}/library/include)

include(nx-utils)

cmake_info("Source Dir : ${GUI_SOURCE_DIR}")

if (cmake_VERBOSE)
    list(LENGTH GUI_SOURCE_FILES GUI_SOURCE_FILES_LENGTH)
    cmake_info("Loaded ${GUI_SOURCE_FILES_LENGTH} source file(s)")
endif ()

file(GLOB SMM_SOURCE_FILES "${SMM_CORE_SOURCE_DIR}/*.cpp")
list(REMOVE_ITEM SMM_SOURCE_FILES ${SMM_CORE_SOURCE_DIR}/main.cpp)

add_executable(${GUI_APP}.elf
        #        ${SOURCE_DIR}/main.cpp
        ${GUI_SOURCE_FILES}
        ${SMM_SOURCE_FILES}
        )

target_include_directories(${GUI_APP}.elf PUBLIC
        ${GUI_INC_DIR}
        ${GUI_INC_DIR}/game_browser
        ${GUI_INC_DIR}/mod_browser
        ${GUI_INC_DIR}/core_extension
        ${BOREALIS_INC_DIR}
        ${BOREALIS_INC_DIR}/borealis
        ${BOREALIS_INC_DIR}/borealis/extern/glad
        ${BOREALIS_INC_DIR}/borealis/extern/nanovg
        ${BOREALIS_INC_DIR}/borealis/extern/libretro-common
#        ${SUBMODULES_DIR}/yaml-cpp/src
#        ${SUBMODULES_DIR}/yaml-cpp/include
        #        ${GUI_INC_DIR}/draw
)

target_link_libraries(
        ${GUI_APP}.elf
        switch::libnx
        -L/opt/devkitpro/portlibs/switch/lib
        -L/opt/devkitpro/libnx/lib
        ${ZLIB_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        -lglfw3 -lEGL -lglapi -ldrm_nouveau -lm -lnx
#        -lSDL2 -march=armv8-a -fPIE -lEGL -lglapi -ldrm_nouveau -lnx
#        -lnx -lfreetype -lSDL2_ttf -lSDL2_gfx -lSDL2_image -lSDL2 -lEGL -lGLESv2 -lglapi -ldrm_nouveau -lpng -ljpeg -L/opt/devkitpro/portlibs/switch/lib -lSDL2 -march=armv8-a -fPIE -L/opt/devkitpro/libnx/lib -lEGL -lglapi -ldrm_nouveau -lnx -L/opt/devkitpro/portlibs/switch/lib -lfreetype -lbz2 -lpng16 -lz -lm -lnx -lz -lwebp
#        -lSDL2  -march=armv8-a -fPIE -lEGL -lglapi -ldrm_nouveau -lSDL2_ttf -lfreetype
#        -lpng -ljpeg -lnx
#        -lfreetype -lSDL2_mixer -lopusfile -lopus -lmodplug -lmpg123 -lvorbisidec -logg -lSDL2_ttf -lSDL2_gfx -lSDL2_image -lSDL2 -lEGL -lGLESv2 -lglapi -ldrm_nouveau -lwebp -lpng -ljpeg
)

set_target_properties(${GUI_APP}.elf PROPERTIES
        LINKER_LANGUAGE CXX # Replace this with C if you have C source files
        LINK_FLAGS "-specs=${LIBNX}/switch.specs -Wl,-no-as-needed -Wl,-Map,.map")


set(CMAKE_BUILD_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${GUI_APP}.elf.dir)
set(BUILD_ROMFS ${CMAKE_BUILD_DIRECTORY}/Resources)
cmake_info("Resources will be gathered in: ${BUILD_ROMFS}")
#add_custom_target(
#        build-time-make-directory ALL
#
#        )
add_custom_command(
        TARGET ${GUI_APP}.elf
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Gathering resources..."
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${BUILD_ROMFS}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_ROMFS}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_ROMFS}/borealis
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${GUI_ROMFS}/. ${BUILD_ROMFS}/.
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${BOREALIS_DIR}/resources/. ${BUILD_ROMFS}/borealis/.
        COMMAND ${CMAKE_COMMAND} -E echo "Resources have been gathered."
)

build_switch_binaries(
        ${GUI_APP}.elf
        ${GUI_TITLE} ${GUI_AUTHOR} ${GUI_ICON} ${APP_VERSION} ${BUILD_ROMFS}
)

