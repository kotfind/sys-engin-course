set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_compile_options(-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "debug")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lower)
if(CMAKE_BUILD_TYPE STREQUAL "release")
    add_compile_options(-g0 -O2)
elseif(CMAKE_BUILD_TYPE STREQUAL "debug")
    add_compile_options(-g3 -O0 -fsanitize=undefined,address)
    add_link_options(-fsanitize=undefined,address)
elseif(CMAKE_BUILD_TYPE STREQUAL "gdb")
    add_compile_options(-g3 -O0)
else()
    message(FATAL_ERROR
        "Unexpected CMAKE_BUILD_TYPE value: `${CMAKE_BUILD_TYPE}`. Supported values: `release`, `debug`, `gdb`")
endif()
