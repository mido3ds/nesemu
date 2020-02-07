cmake_minimum_required(VERSION 3.9)
project(NESemu LANGUAGES CXX VERSION 0.0.1)

# vars
set(BUILD_TYPE "Debug" CACHE STRING "Build type [Release-Debug]")
option(BUILD_TESTS "Enable testing" ON)

## compiler options
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED YES)
    set(CMAKE_CXX_EXTENSIONS NO)

    set(CMAKE_BUILD_TYPE ${BUILD_TYPE})
#

## library
    add_library(libnesemu STATIC)
    set_target_properties(libnesemu PROPERTIES OUTPUT_NAME nesemu)

    ## sources
        file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp src/*.cxx src/*.cc src/*.c)
        file(GLOB_RECURSE MAIN_PATH src/main.*)
        list(REMOVE_ITEM PROJECT_SOURCES "${MAIN_PATH}")

        target_sources(libnesemu PUBLIC ${PROJECT_SOURCES})
    #

    ## headers
        target_include_directories(libnesemu PUBLIC include/)

        # make headers available for IDEs
        file(GLOB_RECURSE PROJECT_HEADERS include/*.hpp include/*.h)
        target_sources(libnesemu PUBLIC ${PROJECT_HEADERS})
    #

    ## dependecies
        # SDL2
        add_subdirectory(vendors/SDL2)
        find_package(SDL2 REQUIRED)
        target_link_libraries(libnesemu PUBLIC SDL2)

        # SDL2_ttf
        include(ExternalProject)
        find_program(MAKE_EXE NAMES gmake nmake make)
        ExternalProject_Add(SDL2_ttf
            SOURCE_DIR "${CMAKE_SOURCE_DIR}/vendors/SDL2_ttf/"
            DOWNLOAD_COMMAND ""
            BUILD_IN_SOURCE YES
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendors/SDL2_ttf/"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ./configure
            COMMAND ${MAKE_EXE}
            INSTALL_COMMAND ""
            BUILD_BYPRODUCTS "${CMAKE_SOURCE_DIR}/vendors/SDL2_ttf/.libs/libSDL2_ttf.so"
        )

        target_link_libraries(libnesemu PUBLIC "${CMAKE_SOURCE_DIR}/vendors/SDL2_ttf/.libs/libSDL2_ttf.so")
        target_sources(libnesemu PUBLIC "${CMAKE_SOURCE_DIR}/vendors/SDL2_ttf/SDL_ttf.h")
    #
#

## executable
    add_executable(nesemu)

    ## sources
        file(GLOB_RECURSE PROJECT_SOURCES src/main.cpp)
        target_sources(nesemu PRIVATE ${PROJECT_SOURCES})
    #

    ## headers
        target_include_directories(nesemu PRIVATE include/)

        # make headers available for IDEs
        file(GLOB_RECURSE PROJECT_HEADERS include/*.hpp include/*.h)
        target_sources(nesemu PRIVATE ${PROJECT_HEADERS})
    #

    ## dependecies
        # libnesemu
        target_link_libraries(nesemu PRIVATE libnesemu)
    #

    # copy files
    file(GLOB FILES_TO_COPY RELATIVE "${CMAKE_SOURCE_DIR}" *.ttf *.nes)
    foreach(FILE_ ${FILES_TO_COPY})
        configure_file(${FILE_} ${FILE_} COPYONLY)
    endforeach()
#

## test
    if(BUILD_TESTS)
        add_executable(test)
        
        file(GLOB_RECURSE TEST_SOURCES test/*.cpp test/*.cxx test/*.cc test/*.c)
        target_sources(test PRIVATE ${TEST_SOURCES})

        ## dependecies
            # libnesemu
            target_link_libraries(test libnesemu)

            # Catch2
            add_subdirectory(vendors/Catch2)
            target_link_libraries(test Catch2::Catch2)
        #
    endif()
#
