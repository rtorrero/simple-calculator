cmake_minimum_required(VERSION 3.10)
project(calculator_project)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Silence warnings 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")

# Create include directory for headers
include_directories(${PROJECT_SOURCE_DIR}/mat_libv2)

# Add executable
add_executable(simple_calculator_v2 
    simple_calculator_v2.cpp
)