CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(argparse-cpp)

IF(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

IF(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
ENDIF()
IF(POLICY CMP0015)
    cmake_policy(SET CMP0015 NEW)
ENDIF()


SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3 -std=c++0x")
SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -O0 -std=c++0x -g -DSWARM_DEBUG")
SET(CMAKE_SHARED_LINKER_FLAGS "-lpcap -dynamiclib")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# INCLUDE_DIRECTORIES(${INC_DIR} ./src)
# LINK_DIRECTORIES(${LIB_DIR})
# IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#     INCLUDE_DIRECTORIES(/usr/local/include)
#     LINK_DIRECTORIES(/usr/local/lib)
# ENDIF()

# Build library

FILE(GLOB BASESRCS "argparse.cc" "argparse.hpp")
FILE(GLOB TESTSRCS "test/*.cc")

ADD_LIBRARY(argparse SHARED ${BASESRCS})
ADD_EXECUTABLE(argparse-test ${TESTSRCS})
TARGET_LINK_LIBRARIES(argparse-test argparse pthread)

ADD_EXECUTABLE(argparse-example example.cc)
TARGET_LINK_LIBRARIES(argparse-example argparse)
