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

# The additional / is important to remove the last character from the path.
# Note that it does not matter if the OS uses / or \, because we are only
# saving the path size.
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

## executable
    add_executable(nesemu)

    ## sources
        file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp src/*.cxx src/*.cc src/*.c)
        target_sources(nesemu PUBLIC ${PROJECT_SOURCES})
    #

    ## headers
        target_include_directories(nesemu PRIVATE include/)

        # make headers available for IDEs
        file(GLOB_RECURSE PROJECT_HEADERS include/*.hpp include/*.h)
        target_sources(nesemu PRIVATE ${PROJECT_HEADERS})
    #

    ## dependecies
        # SFML
        find_package(SFML COMPONENTS window audio system REQUIRED)
        target_link_libraries(nesemu PRIVATE sfml-window sfml-graphics sfml-audio)
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
        
        ## sources
            file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp src/*.cxx src/*.cc src/*.c)
            file(GLOB_RECURSE MAIN_PATH src/main.*)
            list(REMOVE_ITEM PROJECT_SOURCES "${MAIN_PATH}")

            file(GLOB_RECURSE TEST_SOURCES test/*.cpp test/*.cxx test/*.cc test/*.c)
            
            target_sources(test PRIVATE ${TEST_SOURCES} ${PROJECT_SOURCES})
        #
            
        ## headers
            target_include_directories(test PRIVATE test include/)

            file(GLOB_RECURSE TEST_HEADERS test/*.hpp test/*.h)
            target_sources(test PRIVATE ${TEST_HEADERS})
        #

        target_compile_definitions(test PRIVATE -DTEST)

        ## dependecies
            # SFML
            find_package(SFML COMPONENTS window audio system REQUIRED)
            target_link_libraries(test PRIVATE sfml-window sfml-graphics sfml-audio)
        #
    endif()
#
