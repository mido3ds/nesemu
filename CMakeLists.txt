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
        # SFML
        find_package(SFML COMPONENTS window audio system REQUIRED)
        target_link_libraries(libnesemu PRIVATE sfml-window sfml-graphics sfml-audio)
    #
#

## executable
    add_executable(nesemu)

    ## sources
        target_sources(nesemu PRIVATE src/main.cpp)
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

        add_definitions(-DTEST)
    endif()
#
