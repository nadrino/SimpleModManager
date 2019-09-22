file(GLOB_RECURSE
        ALL_CXX_SOURCE_FILES
        *.c *.cpp *.cc *.h *.hpp)

find_program(CLANG_FORMAT "clang-format")
if (CLANG_FORMAT)
    add_custom_target(
            clang_format
            COMMAND /usr/bin/clang-format
            -i
            -style=file
            ${ALL_CXX_SOURCE_FILES})
endif ()

find_program(CLANG_TIDY "clang-tidy")
if (CLANG_TIDY)
    add_custom_target(
            clang-tidy
            COMMAND /usr/bin/clang-tidy
            ${ALL_CXX_SOURCE_FILES}
            -config=''
            --
            -std=c++${CMAKE_CXX_STANDARD}
            ${INCLUDE_DIRECTORIES})
endif ()
