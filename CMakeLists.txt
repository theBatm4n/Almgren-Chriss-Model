cmake_minimum_required(VERSION 3.15)
project(AlmgrenChrissDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable modern C++ features
set(CMAKE_CXX_EXTENSIONS OFF)

# Add executable
add_executable(AlmgrenChrissDemo
    src/main.cpp
    src/trading_engine.cpp
    src/execution_scheduler.cpp
    src/market_impact_model.cpp
)

# Include directories
target_include_directories(AlmgrenChrissDemo PRIVATE include)

# Compiler options
target_compile_options(AlmgrenChrissDemo PRIVATE 
    -Wall 
    -Wextra 
    -Wpedantic
)

# Link libraries
find_package(Threads REQUIRED)
target_link_libraries(AlmgrenChrissDemo Threads::Threads)