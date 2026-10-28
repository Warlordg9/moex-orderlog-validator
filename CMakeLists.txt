cmake_minimum_required(VERSION 3.20)
project(moex_orderlog_validator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(OLV_BUILD_TESTS "Build unit tests" ON)
option(OLV_BUILD_BENCH "Build benchmarks" ON)

# ---- Core library ----
add_library(olv_core
    src/normalize.cpp
    src/price_levels.cpp
    src/order_book.cpp
    src/orderlog_csv.cpp
)
target_include_directories(olv_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(olv_core PRIVATE -Wall -Wextra -Wpedantic)

# ---- CLI ----
add_executable(olv app/main.cpp)
target_link_libraries(olv PRIVATE olv_core)

include(FetchContent)

# ---- Tests (GoogleTest) ----
if(OLV_BUILD_TESTS)
    enable_testing()
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
    add_subdirectory(tests)
endif()

# ---- Benchmarks (Google Benchmark) ----
if(OLV_BUILD_BENCH)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(benchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.9.1.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(benchmark)
    add_subdirectory(bench)
endif()
