add_executable(semaphore_tests
    tests/catch_entry.cpp
    tests/stress.cpp
    tests/unit.cpp
)

target_link_libraries(semaphore_tests Catch2::Catch2)

target_include_directories(semaphore_tests PRIVATE .)
