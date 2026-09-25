cmake_minimum_required(VERSION 3.10)

project(arbotix_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Debug if no build type was specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(
    arbotix_test
    main.cpp
    arbotix_driver.cpp
)

target_include_directories(
    arbotix_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(
    arbotix_test
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)