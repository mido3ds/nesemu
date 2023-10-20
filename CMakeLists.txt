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
    NAME             mu
    GIT_REPOSITORY   https://github.com/mido3ds/mu.git
    GIT_TAG          71d43057bb4846b2da97476c38cb63b9631bb6b9
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

# AppleClang insists on compiling imgui-1.88 with old c++ standards
set_property(TARGET ImGui-SFML
    PROPERTY
        CXX_STANDARD 20
        CMAKE_CXX_STANDARD_REQUIRED YES
        CMAKE_CXX_EXTENSIONS NO
)

add_executable(nesemu)

## sources
target_sources(nesemu
    PRIVATE
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
        src/Config.h
        src/main.cpp
)

## headers
target_include_directories(nesemu PRIVATE src/)

## dependecies
target_link_libraries(nesemu
    PRIVATE
        mido3ds::mu
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
        $<$<CXX_COMPILER_ID:AppleClang>:COMPILER_APPLE_CLANG=1>
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
