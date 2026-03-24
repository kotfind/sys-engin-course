find_program(MOLD_BIN mold)

if(MOLD_BIN)
    message(STATUS "Using mold")
    set(CMAKE_LINKER_TYPE MOLD)
else()
    message(ERROR "Mold not found")
endif()
