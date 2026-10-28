add_executable(olv_bench bench_order_book.cpp)
target_link_libraries(olv_bench PRIVATE olv_core benchmark::benchmark)
