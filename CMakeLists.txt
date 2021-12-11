cmake_minimum_required(VERSION 3.9)

project(Canvas VERSION 0.1 DESCRIPTION "C++ windowing and graphic library for windows")
include(GNUInstallDirs)

if (WIN32)  
    add_library(Canvas source/canvas.cc)
    set_target_properties(Canvas PROPERTIES 
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER include/canvas.hh)

    target_include_directories(Canvas PUBLIC include)

    target_link_libraries(Canvas PRIVATE gdi32)
else()
    message(FATAL_ERROR "Your OS is not supported.")
endif()
