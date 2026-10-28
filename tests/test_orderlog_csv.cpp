#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "olv/order_book.hpp"
#include "olv/orderlog_csv.hpp"

using namespace olv;

TEST(OrderLogCsv, ParsesRowAndSkipsHeader) {
    std::istringstream in(
        "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n"
        "7,SBER,S,100000001,2,2,250.10,3,1001,250.10\r\n");
    const auto r = read_orderlog(in, "0.01");
    ASSERT_EQ(r.events.size(), 1u);
    const Event& e = r.events[0];
    EXPECT_EQ(e.no, 7);
    EXPECT_EQ(e.seccode, "SBER");
    EXPECT_EQ(e.side, Side::Sell);
    EXPECT_EQ(e.action, Action::Fill);
    EXPECT_EQ(e.price_ticks, 25010);
    EXPECT_EQ(e.trade_no, 1001);
    EXPECT_TRUE(r.rejected_lines.empty());
}

TEST(OrderLogCsv, ReportsMalformedLines) {
    std::istringstream in("1,SBER,X,100000000,1,1,250.00,10,,\n1,SBER,B,100000000,1,1\n");
    const auto r = read_orderlog(in, "0.01");
    EXPECT_TRUE(r.events.empty());
    EXPECT_EQ(r.rejected_lines, (std::vector<std::size_t>{1, 2}));
}

TEST(OrderLogCsv, InvalidStepThrows) {
    std::istringstream in;
    EXPECT_THROW(read_orderlog(in, "0"), std::invalid_argument);
}

// End-to-end oracle on the synthetic fixture: exact counts are the contract.
TEST(Replay, SampleFixtureOracle) {
    std::ifstream in(OLV_TEST_DATA_DIR "/sample_orderlog.csv");
    ASSERT_TRUE(in);
    const auto parsed = read_orderlog(in, "0.01");
    EXPECT_EQ(parsed.events.size(), 10u);
    EXPECT_EQ(parsed.rejected_lines, (std::vector<std::size_t>{12}));  // 160.005 is off-grid

    const auto stats = replay(parsed.events);
    ASSERT_EQ(stats.size(), 2u);

    BookStats sber;
    sber.events = 9;
    sber.passed = 6;
    sber.repaired = 1;
    sber.quarantined = 1;
    sber.rejected = 1;
    sber.crossed = 2;
    sber.overfills = 1;
    EXPECT_EQ(stats.at("SBER"), sber);

    BookStats gazp;
    gazp.events = 1;
    gazp.passed = 1;
    EXPECT_EQ(stats.at("GAZP"), gazp);
}
