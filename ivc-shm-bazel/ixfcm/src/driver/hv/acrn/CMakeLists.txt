cmake_minimum_required(VERSION 3.22)

project(ixf_acrnivcdriver CXX)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES COMPILE_FLAGS "-g")

include_directories(./shmem-hal)
include_directories(./memorymanager)

add_subdirectory(ivcreaderwriter)
add_subdirectory(memorymanager)
add_subdirectory(shmem-hal)
add_subdirectory(tests)
