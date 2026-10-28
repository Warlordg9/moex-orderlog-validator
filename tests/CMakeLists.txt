add_executable(olv_tests
    test_normalize.cpp
    test_order_book.cpp
    test_orderlog_csv.cpp
)
target_link_libraries(olv_tests PRIVATE olv_core GTest::gtest_main)
target_compile_definitions(olv_tests PRIVATE OLV_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

include(GoogleTest)
gtest_discover_tests(olv_tests)
