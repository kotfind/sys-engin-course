add_executable(fuse_cpu
    main.cpp
    dynamic_program.cpp
)

target_compile_definitions(fuse_cpu PRIVATE
    # idk, what this is; but fuse explicitly asks to define it
    -D_FILE_OFFSET_BITS=64
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE REQUIRED fuse3)
target_link_libraries(fuse_cpu PRIVATE ${FUSE_LIBRARIES})
target_include_directories(fuse_cpu PRIVATE ${FUSE_INCLUDE_DIRS})
