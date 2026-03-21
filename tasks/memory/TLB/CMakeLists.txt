add_executable(tlb
    main.cpp
    test.cpp
    util.cpp
)

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
if(NOT build_type STREQUAL "gdb")
    message(FATAL_ERROR
        "Only `gdb` build type is supported.\n"
        "This is to prevent optimizations from messing with the test results."
    )
endif()
