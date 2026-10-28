#include <gtest/gtest.h>

#include "olv/normalize.hpp"

using namespace olv;

TEST(ParseFixed, ParsesIntegerAndFraction) {
    EXPECT_EQ(parse_fixed("250"), 250 * kFixedScale);
    EXPECT_EQ(parse_fixed("250.1"), 250 * kFixedScale + 10'000'000);
    EXPECT_EQ(parse_fixed("0.00000001"), 1);
}

TEST(ParseFixed, RejectsMalformed) {
    EXPECT_FALSE(parse_fixed(""));
    EXPECT_FALSE(parse_fixed("abc"));
    EXPECT_FALSE(parse_fixed("1.2.3"));
    EXPECT_FALSE(parse_fixed("-1"));
    EXPECT_FALSE(parse_fixed("1.123456789"));
}

TEST(PriceToTicks, ConvertsOnGridPrices) {
    const auto step = *parse_fixed("0.01");
    EXPECT_EQ(price_to_ticks("250.10", step), 25010);
    EXPECT_EQ(price_to_ticks("0.01", step), 1);
}

TEST(PriceToTicks, RejectsOffGridPrices) {
    const auto step = *parse_fixed("0.01");
    EXPECT_FALSE(price_to_ticks("160.005", step));
    EXPECT_FALSE(price_to_ticks("1", 0));
}

TEST(TimeToNs, ParsesMilliseconds) {
    EXPECT_EQ(time_to_ns("100000001"), 36'000'001'000'000LL);
    EXPECT_EQ(time_to_ns("95959999"), ((9 * 60 + 59) * 60 + 59) * 1'000'000'000LL + 999'000'000LL);
}

TEST(TimeToNs, ParsesMicroseconds) {
    EXPECT_EQ(time_to_ns("100000000001"), 36'000'000'001'000LL);
}

TEST(TimeToNs, RejectsBadInput) {
    EXPECT_FALSE(time_to_ns("250000000"));  // hour 25
    EXPECT_FALSE(time_to_ns("106000000"));  // minute 60
    EXPECT_FALSE(time_to_ns("1000"));
    EXPECT_FALSE(time_to_ns("10:00:00"));
}
