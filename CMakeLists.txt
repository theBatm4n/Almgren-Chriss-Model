cmake_minimum_required(VERSION 3.15)
project(AlmgrenChriss)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build Python bindings
option(BUILD_PYTHON "Build Python bindings" OFF)

# C++ Executable (always built)
add_executable(AlmgrenChrissDemo
    src/main.cpp
    src/trading_engine.cpp
    src/execution_scheduler.cpp
    src/market_impact_model.cpp
)

target_include_directories(AlmgrenChrissDemo PRIVATE include)
target_compile_options(AlmgrenChrissDemo PRIVATE -Wall -Wextra -Wpedantic)
find_package(Threads REQUIRED)
target_link_libraries(AlmgrenChrissDemo PRIVATE Threads::Threads)

# Python bindings (optional)
if(BUILD_PYTHON)
    find_package(pybind11 REQUIRED)
    
    pybind11_add_module(almgren_chriss
        python/bindings.cpp
        src/trading_engine.cpp
        src/execution_scheduler.cpp
        src/market_impact_model.cpp
    )
    
    target_include_directories(almgren_chriss PRIVATE include)
    target_link_libraries(almgren_chriss PRIVATE Threads::Threads)
endif()