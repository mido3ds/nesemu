cmake_minimum_required (VERSION 2.8.11)
project (NesEmu)

#
# CMake setup
#
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set (CMAKE_VERBOSE_MAKEFILE 0) # 1 should be used for debugging
set (CMAKE_SUPPRESS_REGENERATION TRUE) # Suppresses ZERO_CHECK
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
endif()

#
# Add files
#
file (GLOB_RECURSE PROJECT_HEADERS src/*.hpp
                                   src/*.h)
file (GLOB_RECURSE PROJECT_SOURCES src/*.cpp
                                   src/*.cxx
                                   src/*.cc
                                   src/*.c)
file (GLOB         PROJECT_CONFIGS CMakeLists.txt)

#
# Organizing files
#
source_group ("headers" FILES ${PROJECT_HEADERS})
source_group ("sources" FILES ${PROJECT_SOURCES})

#
# Set executable and target link libraries
#
add_executable (${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                                ${PROJECT_CONFIGS} ${VENDORS_SOURCES})
set_target_properties (${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

#
# add SDL2
# 
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})

#
# add yaml-cpp
#
find_package(PkgConfig)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp>=0.5)
include_directories(${YAMLCPP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${YAMLCPP_LIBRARIES})

#
# create makefile
#
file (WRITE Makefile "# auto-generated makefile

.PHONY: all cmake build run

all: cmake build run

cmake:
\tcmake -S. -B\"${CMAKE_BINARY_DIR}\" || echo \">>>> cmake initialization failed! <<<<<\"

build:
\tcmake --build \"${CMAKE_BINARY_DIR}\" || echo \">>>> build failed! <<<<<\"

run:
\t\"${CMAKE_BINARY_DIR}/${PROJECT_NAME}/Debug/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}\" mario.nes config.yaml || \"${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}\" mario.nes config.yaml

clean:
\tcmake --build \"${CMAKE_BINARY_DIR}\" --target clean || echo \">>>> clean failed! <<<<<\"

cleanAll:
\t      del Makefile && rd /s /q build && rd /s /q CMakeFiles || rm -rf build CMakeFiles Makefile
")
