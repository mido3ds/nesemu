cmake_minimum_required(VERSION 3.9)
project(NESemu LANGUAGES CXX VERSION 0.0.1)

option(NESEMU_PEDANTIC_BUILD "Enable pedantic warnings during build" OFF)

if (CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE Debug)
endif()

if (CPM_SOURCE_CACHE STREQUAL "")
        message(WARNING "CPM_SOURCE_CACHE is empty, this will download locally all packages")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_RPATH "$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(BUILD_TESTS "Enable testing" ON)

# dependencies
include(CPM)
CPMAddPackage(
	NAME             sfml
	GIT_REPOSITORY   https://github.com/SFML/SFML
	GIT_TAG          2.5.1
	GIT_SHALLOW      TRUE
)
CPMAddPackage(
	NAME             freetype
	GIT_REPOSITORY   https://github.com/freetype/freetype.git
	GIT_TAG          VER-2-11-1
	GIT_SHALLOW      TRUE
)
CPMAddPackage(
    NAME             Catch2
    GIT_REPOSITORY   https://github.com/catchorg/Catch2.git
    GIT_TAG          v2.13.8
    GIT_SHALLOW      TRUE
)

# The additional / is important to remove the last character from the path.
# Note that it does not matter if the OS uses / or \, because we are only
# saving the path size.
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

add_executable(nesemu)

## sources
file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp src/*.cxx src/*.cc src/*.c)
target_sources(nesemu PUBLIC ${PROJECT_SOURCES})

## headers
target_include_directories(nesemu PRIVATE include/)

# make headers available for IDEs
file(GLOB_RECURSE PROJECT_HEADERS include/*.hpp include/*.h)
target_sources(nesemu PRIVATE ${PROJECT_HEADERS})

## dependecies
target_link_libraries(nesemu PRIVATE sfml-window sfml-graphics sfml-audio freetype Catch2)

# copy files
file(GLOB FILES_TO_COPY RELATIVE "${CMAKE_SOURCE_DIR}" *.ttf *.nes)
foreach(FILE_ ${FILES_TO_COPY})
    configure_file(${FILE_} ${FILE_} COPYONLY)
endforeach()

set_property(TARGET nesemu
    PROPERTY
        CXX_STANDARD 20
        CMAKE_CXX_STANDARD_REQUIRED YES
        CMAKE_CXX_EXTENSIONS NO
)

target_compile_definitions(nesemu
    PRIVATE
        ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

# TODO: why is this? maybe because of nestestlines.cpp? if so, then i have to remove it
# target_compile_options(nesemu PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)

if (${NESEMU_PEDANTIC_BUILD})
    target_compile_options(nesemu
            PRIVATE
                    $<$<CXX_COMPILER_ID:MSVC>:/W4 /NODEFAULTLIB:library>
                    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Wno-nested-anon-types>
    )
endif()
