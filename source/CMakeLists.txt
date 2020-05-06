set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/source)

file(GLOB SOURCE_FILES "${SOURCE_DIR}/*.cpp")

cmake_info("Source Dir : ${SOURCE_DIR}")

if (cmake_VERBOSE)
    list(LENGTH SOURCE_FILES SOURCE_FILES_LENGTH)
    cmake_info("Loaded ${SOURCE_FILES_LENGTH} source file(s)")
endif ()

add_executable(${HOMEBREW_APP}.elf
#        ${SOURCE_DIR}/main.cpp
        ${SOURCE_FILES}
        )

target_link_libraries(
        ${HOMEBREW_APP}.elf
        switch::libnx
        -L/opt/devkitpro/portlibs/switch/lib
        ${ZLIB_LIBRARIES}
        #        -lSDL2 -march=armv8-a -fPIE -L/opt/devkitpro/libnx/lib -lEGL -lglapi -ldrm_nouveau -lnx
)
set_target_properties(${HOMEBREW_APP}.elf PROPERTIES
        LINKER_LANGUAGE CXX # Replace this with C if you have C source files
        LINK_FLAGS "-specs=${LIBNX}/switch.specs -Wl,-no-as-needed -Wl,-Map,.map")

build_switch_binaries(${HOMEBREW_APP}.elf)