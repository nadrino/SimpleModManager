###########################
# Definition of Borealis lib build
###########################

set( BOREALIS_DIR ${SUBMODULES_DIR}/borealis )
set(BOREALIS_INC_DIR ${BOREALIS_DIR}/library/include)

cmake_info("BOREALIS_DIR is ${BOREALIS_DIR}")
add_compile_definitions( BOREALIS_RESOURCES=\"romfs:/borealis/\" )


file( GLOB BOREALIS_SRC
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
    "${BOREALIS_DIR}/library/lib/extern/fmt/src/*.cc"
    )

add_library( Borealis STATIC ${BOREALIS_SRC} )
install( TARGETS Borealis DESTINATION lib )

target_include_directories( Borealis PUBLIC
    ${BOREALIS_DIR}/library/lib/extern/fmt/include
    ${BOREALIS_INC_DIR}
    ${BOREALIS_INC_DIR}/borealis
    ${BOREALIS_INC_DIR}/borealis/extern
    ${BOREALIS_INC_DIR}/borealis/extern/glad
    ${BOREALIS_INC_DIR}/borealis/extern/nanovg
    ${BOREALIS_INC_DIR}/borealis/extern/libretro-common
    ${PROJECT_SOURCE_DIR}/shortcuts
    ${PROJECT_SOURCE_DIR}/shortcuts/libretro-common
#    ${SUBMODULES_DIR}/json/include
    )

target_link_libraries( Borealis PUBLIC
    switch::libnx
    -L/opt/devkitpro/portlibs/switch/lib
    -L/opt/devkitpro/libnx/lib
    ${ZLIB_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    -lglfw3 -lEGL -lglad -lglapi -ldrm_nouveau -lm -lnx
    )




