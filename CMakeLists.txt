cmake_minimum_required(VERSION 3.9)
project(NESemu LANGUAGES CXX VERSION 0.0.1)

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
		target_link_libraries(nesemu PRIVATE sfml-window sfml-graphics sfml-audio freetype)
    #

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
            target_link_libraries(test PRIVATE sfml-window sfml-graphics sfml-audio)
        #

		set_property(TARGET test
			PROPERTY
				CXX_STANDARD 20
				CMAKE_CXX_STANDARD_REQUIRED YES
				CMAKE_CXX_EXTENSIONS NO
		)

		target_compile_options(test PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)
    endif()
#
