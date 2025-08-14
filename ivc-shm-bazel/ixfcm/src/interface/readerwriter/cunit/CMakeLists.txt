cmake_minimum_required(VERSION 3.22)

project(ivc-cuint-test CXX)

FILE(GLOB SRCFILES *.cpp)

include_directories (.)

find_library(CUNIT_LIB cunit)
find_path(CUNIT_INCLUDE_DIR NAMES CUnit/CUnit.h)

if(NOT CUNIT_LIB OR NOT CUNIT_INCLUDE_DIR)
	message(FATAL_ERROR "CUnit headers are required but could not found. Please install CUnit.")
endif()

foreach( testsourcefile ${SRCFILES} )
    get_filename_component( testname ${testsourcefile} NAME_WE )
    add_executable( ${testname} ${testsourcefile} )
    target_link_libraries( ${testname} ixfcm_ivcreaderwriter )
    target_link_libraries(${testname} libcunit.so)
    install(TARGETS ${testname} DESTINATION ${CMAKE_INSTALL_BINDIR})
    set_target_properties(${testname} PROPERTIES COMPILE_FLAGS "-g")
endforeach( testsourcefile ${SRCFILES} )

enable_testing()
#add_test(NAME test COMMAND test_hello)




