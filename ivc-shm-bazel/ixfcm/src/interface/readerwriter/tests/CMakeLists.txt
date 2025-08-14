cmake_minimum_required(VERSION 3.22)

project(ixf_ivcreaderwriter_test CXX)

FILE(GLOB SRCFILES *.cpp)

include_directories (.)

foreach( testsourcefile ${SRCFILES} )
    get_filename_component( testname ${testsourcefile} NAME_WE )
    add_executable( ${testname} ${testsourcefile} )
    target_link_libraries( ${testname} ixfcm_ivcreaderwriter )
    #target_link_libraries( ${testname} ixfcm_ivcmetaaccess )
    install(TARGETS ${testname} DESTINATION ${CMAKE_INSTALL_BINDIR})
    set_target_properties(${testname} PROPERTIES COMPILE_FLAGS "-g")
endforeach( testsourcefile ${SRCFILES} )

