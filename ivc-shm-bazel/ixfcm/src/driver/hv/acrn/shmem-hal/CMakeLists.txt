cmake_minimum_required(VERSION 3.22)

project(ixfcm_acrn_shmemhal CXX)

set(LIB_NAME ixfcm_acrn_shmemhal)

FILE(GLOB SRCFILES *.cpp)

include_directories (.)

add_library(${LIB_NAME} STATIC ${SRCFILES})

target_link_libraries (${LIB_NAME}
    pthread
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES COMPILE_FLAGS "-fPIC -g -fpermissive")

install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
