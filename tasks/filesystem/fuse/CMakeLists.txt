
# -------------------- General --------------------

add_executable(fuse_cpu
    main.cpp
    program.cpp
    unit.cpp
)

# -------------------- Fuse --------------------

find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE REQUIRED fuse3)
target_link_libraries(fuse_cpu PRIVATE ${FUSE_LIBRARIES})
target_include_directories(fuse_cpu PRIVATE ${FUSE_INCLUDE_DIRS})

target_compile_definitions(fuse_cpu PRIVATE
    # idk, what this is; but fuse explicitly asks to define it
    -D_FILE_OFFSET_BITS=64
)

# -------------------- Doxygen --------------------

find_package(Doxygen REQUIRED)
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs/fuse_cpu")
doxygen_add_docs(fuse_cpu_docs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ALL
)
