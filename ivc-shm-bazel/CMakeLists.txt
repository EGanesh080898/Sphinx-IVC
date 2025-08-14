cmake_minimum_required(VERSION 3.22)

project(ivcshm VERSION 1.0.0 LANGUAGES CXX)

set(HV_PLATFORM "ACRN")


if(HV_PLATFORM STREQUAL "ACRN")
    message("Platform set is ACRN")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -rdynamic -DACRN_HV_PLATFORM -DACRN_ENABLE_IVCHEARTBEAT")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -rdynamic -DACRN_HV_PLATFORM")
endif()
if(HV_PLATFORM STREQUAL "QNX")
    message("Platform set is QNX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -rdynamic -DQNX_HV_PLATFORM")
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INCLUDE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/inc)

set(CMAKE_C_FLAGS_DEBUG_INIT "-Wall -g")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -g")
set(CMAKE_C_FLAGS_RELEASE_INIT "-Wall -s")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall -s")

set(LOGGER_INCLUDE "${CMAKE_HOME_DIRECTORY}/logger")
include_directories(${LOGGER_INCLUDE})
set(READERWRITER_INTERFACE_INCLUDE "${CMAKE_HOME_DIRECTORY}/ixfcm/src/driver/interface/readerwriter")
include_directories(${READERWRITER_INTERFACE_INCLUDE})
#set(METAACCESS_INTERFACE_INCLUDE "${CMAKE_HOME_DIRECTORY}/ixfcm/src/driver/interface/metaaccess")
#include_directories(${METAACCESS_INTERFACE_INCLUDE})
add_subdirectory(ixfcm)
add_subdirectory(logger)
#add_subdirectory(ivccore)
