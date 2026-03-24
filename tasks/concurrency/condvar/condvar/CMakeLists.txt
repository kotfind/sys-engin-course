add_executable(condvar_tests
    tests/catch_entry.cpp
    tests/stress.cpp
    tests/unit.cpp
)

target_link_libraries(condvar_tests Catch2::Catch2)
