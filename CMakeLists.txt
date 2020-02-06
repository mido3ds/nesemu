cmake_minimum_required(VERSION 2.8.11)
project(nesemu)

# include /cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

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

## compiler options
    set_target_properties(nesemu PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )

    set(BUILD_TYPE "Debug" CACHE STRING "build type")
    set(CMAKE_BUILD_TYPE ${BUILD_TYPE})
#