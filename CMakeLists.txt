cmake_minimum_required(VERSION 3.9)

project(Canvas VERSION 0.1 DESCRIPTION "C++ windowing and graphic library for windows")

file(GLOB_RECURSE SOURCE "source/*.cc")


if (WIN32)  
    add_library(Canvas SHARED ${SOURCE})
    set_target_properties(Canvas PROPERTIES VERSION ${PROJECT_VERSION})
    set_target_properties(Canvas VERSION ${PROJECT_VERSION})
    set_target_properties(Canvas PROPERTIES PUBLIC_HEADER "include/canvas.hh")

    target_include_directories(Canvas PRIVATE include)
    target_include_directories(Canvas PRIVATE source)

    install(TARGETS Canvas
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
else()i
    message(FATAL_ERROR "Your OS is not supported.")
endif()
