
# -------------------- General --------------------

add_executable(fuse_cpu
    cpu/cpu.cpp
    cpu/program.cpp
    cpu/unit.cpp

    fuse_core/fuse_dir.cpp
    fuse_core/fuse_file.cpp
    fuse_core/fuse_fs.cpp

    fuse_cpu/ctrl_file.cpp
    fuse_cpu/fuse_cpu.cpp
    fuse_cpu/unit_data_file.cpp
    fuse_cpu/unit_program_file.cpp

    args.cpp
    main.cpp
    run.cpp
    signal_handler.cpp
)

target_include_directories(fuse_cpu PRIVATE
    .
    ./cpu
    ./fuse_core
    ./fuse_cpu
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

set(DOXYGEN_PROJECT_NAME "Fuse Cpu")
set(DOXYGEN_PROJECT_BRIEF "Create a 'virtual cpu' with FUSE filesystem interface.")
set(DOXYGEN_PROJECT_NUMBER "v0.1.0")

set(DOXYGEN_EXTRACT_ALL true)
set(DOXYGEN_EXCLUDE_PATTERNS "*.cpp")
set(DOXYGEN_EXCLUDE
    integration
    README.md
)

set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs/fuse_cpu")

doxygen_add_docs(fuse_cpu_docs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ALL
)

# -------------------- Install --------------------

install(TARGETS fuse_cpu
    RUNTIME DESTINATION bin
    COMPONENT app
)

install(DIRECTORY "${CMAKE_BINARY_DIR}/docs/fuse_cpu/html/"
    DESTINATION share/doc/fuse_cpu
    COMPONENT docs
)
