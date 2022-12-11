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

# dependencies
include(CPM)
CPMAddPackage(
	NAME             fmt
	GIT_REPOSITORY   https://github.com/fmtlib/fmt.git
	GIT_TAG          8.1.1
	GIT_SHALLOW      TRUE
)
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
CPMAddPackage(
    NAME             ImGui-SFML
    GIT_REPOSITORY   https://github.com/eliasdaler/imgui-sfml
    GIT_TAG          v2.5
    GIT_SHALLOW      TRUE
    OPTIONS
        "IMGUI_SFML_IMGUI_DEMO ON"
        "IMGUI_SFML_FIND_SFML OFF"
        "IMGUI_DIR ${CMAKE_SOURCE_DIR}/extern/imgui-1.88"
)

# The additional / is important to remove the last character from the path.
# Note that it does not matter if the OS uses / or \, because we are only
# saving the path size.
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

add_executable(nesemu)

## sources
target_sources(nesemu
    PRIVATE
        src/utils.cpp
        src/test/single_instructions.cpp
        src/test/nestestlines.cpp
        src/test/nestest.h
        src/test/run_tests.cpp
        src/test/nestest.cpp
        src/Image.cpp
        src/Image.h
        src/common.h
        src/instructions.h
        src/Console.h
        src/Console.cpp
        src/ROM.cpp
        src/RAM.h
        src/CPU.h
        src/instructions.cpp
        src/ROM.h
        src/PPU.cpp
        src/PPU.h
        src/CPU.cpp
        src/common.cpp
        src/RAM.cpp
        src/utils.h
        src/Config.h
        src/main.cpp
)

## headers
target_include_directories(nesemu PRIVATE src/)

## dependecies
target_link_libraries(nesemu
    PRIVATE
        fmt
        sfml-window
        sfml-graphics
        sfml-audio
        freetype
        Catch2
        ImGui-SFML::ImGui-SFML
	    $<$<PLATFORM_ID:Windows>:dbghelp>
)

set_property(TARGET nesemu
    PROPERTY
        CXX_STANDARD 20
        CMAKE_CXX_STANDARD_REQUIRED YES
        CMAKE_CXX_EXTENSIONS NO
)

target_compile_definitions(nesemu
    PRIVATE
        ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
        $<$<PLATFORM_ID:Windows>:UNICODE;_UNICODE>
		$<$<PLATFORM_ID:Windows>:OS_WINDOWS=1>
		$<$<PLATFORM_ID:Linux>:OS_LINUX=1>
		$<$<PLATFORM_ID:Darwin>:OS_MACOS=1>
		$<$<CXX_COMPILER_ID:Clang>:COMPILER_CLANG=1>
		$<$<CXX_COMPILER_ID:AppleClang>:COMPILER_CLANG=1>
		$<$<CXX_COMPILER_ID:GNU>:COMPILER_GNU=1>
		$<$<CXX_COMPILER_ID:MSVC>:COMPILER_MSVC=1>
		$<$<CONFIG:DEBUG>:DEBUG>
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
