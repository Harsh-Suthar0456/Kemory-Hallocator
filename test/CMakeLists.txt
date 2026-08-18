include(GoogleTest)

add_executable(unit_test
    buddy_allocator_test.cc
)

target_link_libraries(unit_test
    PRIVATE 
        khalloc
        GTest::gtest_main
)

target_include_directories(unit_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

gtest_discover_tests(unit_test)