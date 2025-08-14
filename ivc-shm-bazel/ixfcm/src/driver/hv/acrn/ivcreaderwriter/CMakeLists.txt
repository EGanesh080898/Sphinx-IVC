cmake_minimum_required(VERSION 3.22)

set(LIB_NAME ixfcm_acrn_ivcreaderwriter)

FILE(GLOB SRCFILES
    *.cpp )

include_directories (.)
include_directories (${READERWRITER_INTERFACE_INCLUDE})

add_library(${LIB_NAME} STATIC ${SRCFILES})

target_link_libraries (${LIB_NAME}
    pthread
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES COMPILE_FLAGS "-fPIC -g")

install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
