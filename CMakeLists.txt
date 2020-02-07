cmake_minimum_required(VERSION 3.9)
project(nesemu LANGUAGES CXX VERSION 0.0.1)

# vars
set(BUILD_TYPE "Debug" CACHE STRING "build type")
option(GIT_SUBMODULE "Check submodules during build" ON)
option(ENABLE_BUILD_TESTS "enable testing" ON)

# include /cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

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

## executable target
add_executable(nesemu)

## sources
    file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp src/*.cxx src/*.cc src/*.c)
    target_sources(nesemu PRIVATE ${PROJECT_SOURCES})
#

## headers
    target_include_directories(nesemu PRIVATE include/)

    # make headers available for IDEs
    file(GLOB_RECURSE PROJECT_HEADERS include/*.hpp include/*.h)
    target_sources(nesemu PRIVATE ${PROJECT_HEADERS})
#

## dependecies
    # SDL2
    find_package(SDL2 REQUIRED)
    target_include_directories(nesemu PRIVATE "${SDL2_INCLUDE_DIR}")
    target_link_libraries(nesemu      PRIVATE ${SDL2_LIBRARY})

    # SDL2_ttf
    find_package(SDL2_ttf REQUIRED)
    target_include_directories(nesemu PRIVATE "${SDL2_TTF_INCLUDE_DIR}")
    target_link_libraries(nesemu      PRIVATE ${SDL2_TTF_LIBRARIES})
#

## tests
    if(ENABLE_BUILD_TESTS)
        include(CTest)

        # Catch2
        if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/Catch2/CMakeLists.txt")
            message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
        else()
            set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/extern/Catch2/contrib")
            add_subdirectory(extern/Catch2)
            include(Catch)
        endif()
        
        # tests

        file(GLOB_RECURSE TEST_SOURCES RELATIVE "${PROJECT_SOURCE_DIR}/test" test/*.cpp test/*.cxx test/*.cc test/*.c)
        foreach(FILENAME ${TEST_SOURCES})
            string(REPLACE ".cpp" "" FILENAME2 ${FILENAME})
            string(REPLACE ".cxx" "" FILENAME2 ${FILENAME2})
            string(REPLACE ".c" "" FILENAME2 ${FILENAME2})
            string(REPLACE ".cc" "" FILENAME2 ${FILENAME2})

            message(STATUS "Add test file ${FILENAME} with name ${FILENAME2}")

            add_executable(${FILENAME2} test/${FILENAME})
            target_link_libraries(${FILENAME2} Catch2::Catch2)

            catch_discover_tests(${FILENAME2})
        endforeach()
        
    endif()
#

## compiler options
    set_target_properties(nesemu PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )

    set(CMAKE_BUILD_TYPE ${BUILD_TYPE})

    # link time optimization
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result)
        set_target_properties(nesemu PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
#