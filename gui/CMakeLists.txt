#
# CMakeLists for the Gui App
#

# Define NRO builder tools
include( ${CMAKE_SOURCE_DIR}/cmake/nx-utils.cmake )

# Define Borealis library
include( ${CMAKE_SOURCE_DIR}/cmake/borealisLib.cmake )


add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/CoreExtension )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/ModBrowser )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/GameBrowser )

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/Applications )





cmake_info( "Source Dir : ${GUI_SOURCE_DIR}" )


file(GLOB GUI_SOURCE_FILES
        "${GUI_SOURCE_DIR}/*.cpp"
        "${GUI_SOURCE_DIR}/game_browser/*.cpp"
        "${GUI_SOURCE_DIR}/mod_browser/*.cpp"
        "${GUI_SOURCE_DIR}/core_extension/*.cpp"
    )


if (cmake_VERBOSE)
    list(LENGTH GUI_SOURCE_FILES GUI_SOURCE_FILES_LENGTH)
    cmake_info("Loaded ${GUI_SOURCE_FILES_LENGTH} source file(s)")
endif ()

#file(GLOB SMM_SOURCE_FILES "${SMM_CORE_SOURCE_DIR}/*.cpp")
#list(REMOVE_ITEM SMM_SOURCE_FILES ${SMM_CORE_SOURCE_DIR}/main.cpp)
