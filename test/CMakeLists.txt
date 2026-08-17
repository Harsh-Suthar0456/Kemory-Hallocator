find_package(Catch2 3 REQUIRED)

include(Catch)

add_executable(unit_test
    segregated_test.cc
)

target_link_libraries(unit_test
    PRIVATE 
        khalloc
        Catch2::Catch2WithMain
)

target_include_directories(unit_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

catch_discover_tests(unit_test)