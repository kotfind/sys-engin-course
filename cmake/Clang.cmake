# should be included before `project(...)`

find_program(CLANG_BIN clang)
find_program(CLANGXX_BIN clang++)

if(NOT CLANG_BIN)
    message(FATAL_ERROR "Clang not found")
endif()

if(NOT CLANGXX_BIN)
    message(FATAL_ERROR "Clang++ not found")
endif()

set(CMAKE_C_COMPILER ${CLANG_BIN})
set(CMAKE_CXX_COMPILER ${CLANGXX_BIN})

