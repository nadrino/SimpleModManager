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
