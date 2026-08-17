cmake_minimum_required(VERSION 3.20)

project(khalloc
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(khalloc STATIC
    src/khalloc.cc
    src/segregated.cc
)

target_include_directories(khalloc
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()