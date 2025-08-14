cmake_minimum_required(VERSION 3.22)

project(ixf_ivcreaderwriter CXX)

set(LIB_NAME ixfcm_ivcreaderwriter)

FILE(GLOB SRCFILES *.cpp)

include_directories (.)
include_directories(${READERWRITER_INTERFACE_INCLUDE})

add_library(${LIB_NAME} SHARED ${SRCFILES})

find_library(JSONCPP_LIBRARY NAMES jsoncpp PATHS ${JSONCPP_DIR})
if(NOT JSONCPP_LIBRARY)
    message(FATAL_ERROR "Jsoncpp library not found. Please install jsoncpp.")
endif()

target_link_libraries (${LIB_NAME}
    ixfcm_acrn_ivcreaderwriter
    ixfcm_acrn_memorymanager
    ixfcm_acrn_shmemhal
    ${JSONCPP_LIBRARY}  # Link against jsoncpp.so
)

target_sources(${LIB_NAME} PUBLIC IvcReaderWriter.hpp)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
GET_TARGET_PROPERTY(MYLIB_PUBLIC_HEADERS ${LIB_NAME} INTERFACE_SOURCES)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PUBLIC_HEADER ${MYLIB_PUBLIC_HEADERS})

install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS ${LIB_NAME} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(tests)
add_subdirectory(cunit)
