cmake_minimum_required(VERSION 3.10)
project(ArithmeticCoder)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -O2)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

set(SOURCES
    src/arithmetic_coder.cpp
    src/bit_io.cpp
    src/main.cpp
    src/utils.cpp
)

# Create executable
add_executable(arithmetic_coder ${SOURCES})