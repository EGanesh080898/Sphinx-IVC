cmake_minimum_required(VERSION 3.22)

if(HV_PLATFORM STREQUAL "ACRN")
    message("Driver set to ACRN")
    add_subdirectory(driver/hv/acrn)
endif()
if(HV_PLATFORM STREQUAL "QNX")
    message("Driver set to QNX")
#    add_subdirectory(driver/hv/qnx)
endif()

add_subdirectory(interface)
