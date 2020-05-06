#
# CMakeLists for the Tesla Overlay
#

# Replace this with the name of your application
set(OVL_NAME "SimpleModManagerOverlay")
set(OVL_APP "${OVL_NAME}_${VERSION_MAJOR}_${VERSION_MINOR}_${VERSION_MICRO}")

# Meta information about the app
set(OVL_TITLE ${OVL_NAME})
set(OVL_AUTHOR "Nadrino")
set(OVL_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.jpg")


set(OVL_SOURCE_DIR ${PROJECT_SOURCE_DIR}/ovl/source)

file(GLOB OVL_SOURCE_FILES "${OVL_SOURCE_DIR}/*.cpp")
file(GLOB OVL_TESLA_FILES "${PROJECT_SOURCE_DIR}/ovl/libs/libtesla/include/*.hpp" "${PROJECT_SOURCE_DIR}/ovl/libs/libtesla/include/*.h")

include_directories("${PROJECT_SOURCE_DIR}/ovl/libs/libtesla/include/")
include(nx-utils)

cmake_info("Source Dir : ${OVL_SOURCE_DIR}")
#cmake_info("Tesla files : ${OVL_TESLA_FILES}")

##
## Removes the specified compile flag from the specified target.
##   _target     - The target to remove the compile flag from
##   _flag       - The compile flag to remove
##
## Pre: apply_global_cxx_flags_to_all_targets() must be invoked.
##
#macro(remove_flag_from_target _target _flag)
#    get_target_property(_target_cxx_flags ${_target} COMPILE_OPTIONS)
#    if(_target_cxx_flags)
#        list(REMOVE_ITEM _target_cxx_flags ${_flag})
#        set_target_properties(${_target} PROPERTIES COMPILE_OPTIONS "${_target_cxx_flags}")
#    endif()
#endmacro()
#
##
## Applies CMAKE_CXX_FLAGS to all targets in the current CMake directory.
## After this operation, CMAKE_CXX_FLAGS is cleared.
##
#macro(apply_global_cxx_flags_to_all_targets)
#    separate_arguments(_global_cxx_flags_list UNIX_COMMAND ${CMAKE_CXX_FLAGS})
#    get_property(_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
#    foreach(_target ${_targets})
#        target_compile_options(${_target} PUBLIC ${_global_cxx_flags_list})
#    endforeach()
#    unset(CMAKE_CXX_FLAGS)
#    set(_flag_sync_required TRUE)
#endmacro()


if (cmake_VERBOSE)
    list(LENGTH OVL_SOURCE_FILES OVL_SOURCE_FILES_LENGTH)
    cmake_info("Loaded ${OVL_SOURCE_FILES_LENGTH} source file(s)")
endif ()

set(SMM_SOURCES ${SOURCE_FILES})
list(REMOVE_ITEM SMM_SOURCES ${PROJECT_SOURCE_DIR}/source/main.cpp)

add_executable(${OVL_APP}.elf
        #        ${SOURCE_DIR}/main.cpp
        ${OVL_SOURCE_FILES}
        ${OVL_TESLA_FILES}
        ${SMM_SOURCES}
        )

target_link_libraries(
        ${OVL_APP}.elf
        switch::libnx
        -L/opt/devkitpro/portlibs/switch/lib
        ${ZLIB_LIBRARIES}
        #        -lSDL2 -march=armv8-a -fPIE -L/opt/devkitpro/libnx/lib -lEGL -lglapi -ldrm_nouveau -lnx
)

set_target_properties(${OVL_APP}.elf PROPERTIES
        LINKER_LANGUAGE CXX # Replace this with C if you have C source files
        LINK_FLAGS "-specs=${LIBNX}/switch.specs -Wl,-no-as-needed -Wl,-Map,.map")

build_switch_binaries(${OVL_APP}.elf)




