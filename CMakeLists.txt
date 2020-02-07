cmake_minimum_required(VERSION 3.9)
project(nesemu LANGUAGES CXX VERSION 0.0.1)

# vars
set(BUILD_TYPE "Debug" CACHE STRING "Build type [Release-Debug]")
option(GIT_SUBMODULE "Check submodules during build" ON)
option(BUILD_TESTS "Enable testing" ON)

# submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

## compiler options
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED YES)
    set(CMAKE_CXX_EXTENSIONS NO)

    set(CMAKE_BUILD_TYPE ${BUILD_TYPE})

    # link time optimization
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
#

## library
    add_library(libnesemu)
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
        # include cmake/FindSDL2*.cmake files
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
        
        # SDL2
        find_package(SDL2 REQUIRED)
        target_include_directories(libnesemu PUBLIC "${SDL2_INCLUDE_DIR}")
        target_link_libraries(libnesemu      PUBLIC ${SDL2_LIBRARY})

        # SDL2_ttf
        find_package(SDL2_ttf REQUIRED)
        target_include_directories(libnesemu PUBLIC "${SDL2_TTF_INCLUDE_DIR}")
        target_link_libraries(libnesemu      PUBLIC ${SDL2_TTF_LIBRARIES})
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
            add_subdirectory(extern/Catch2)
            target_link_libraries(test Catch2::Catch2)
        #
    endif()
#
