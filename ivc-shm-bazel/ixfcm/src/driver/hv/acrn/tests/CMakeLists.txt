cmake_minimum_required(VERSION 3.22)

project(ixf_acrndriver_test CXX)

FILE(GLOB SRCFILES *.cpp)

include_directories (.)

foreach( testsourcefile ${SRCFILES} )
    get_filename_component( testname ${testsourcefile} NAME_WE )
    add_executable( ${testname} ${testsourcefile} )
    target_link_libraries( ${testname} ixfcm_ivcreaderwriter )
    install(TARGETS ${testname} DESTINATION ${CMAKE_INSTALL_BINDIR})
    SET_TARGET_PROPERTIES(${testname} PROPERTIES COMPILE_FLAGS "-fpermissive -g")
endforeach( testsourcefile ${SRCFILES} )

