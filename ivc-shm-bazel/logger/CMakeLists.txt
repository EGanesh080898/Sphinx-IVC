cmake_minimum_required(VERSION 3.22)
project(MyLibrary VERSION 1.0.0 LANGUAGES CXX)

# Create an interface library
add_library(MyLibrary INTERFACE)

# Set the include directories for the library
target_include_directories(MyLibrary INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Optionally, if you want to install the header file
install(FILES ivcLog.hpp DESTINATION include)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ivcLog.h DESTINATION ${CMAKE_CURRENT_LIST_DIR}/../build/inc)
