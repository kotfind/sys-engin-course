cmake_minimum_required(VERSION 3.13)

include(cmake/Clang.cmake)
project(sys_engin_course)

find_package(Catch2 REQUIRED)

include(cmake/Ccache.cmake)
include(cmake/Flags.cmake)
include(cmake/Mold.cmake)

add_subdirectory(tasks)
