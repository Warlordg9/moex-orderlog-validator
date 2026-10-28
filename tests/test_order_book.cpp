#include <gtest/gtest.h>

#include "olv/order_book.hpp"

using namespace olv;

namespace {

Event ev(Action a, Side s, std::int64_t order_no, std::int64_t price, std::int64_t volume) {
    Event e;
    e.seccode = "TEST";
    e.action = a;
    e.side = s;
    e.order_no = order_no;
    e.price_ticks = price;
    e.volume = volume;
    return e;
}

}  // namespace

template <typename Levels>
class PriceLevelsTest : public ::testing::Test {};
using LevelImpls = ::testing::Types<MapLevels, VectorLevels>;
TYPED_TEST_SUITE(PriceLevelsTest, LevelImpls);

TYPED_TEST(PriceLevelsTest, BidsBestIsHighest) {
    TypeParam bids(Side::Buy);
    bids.add(100, 1);
    bids.add(102, 2);
    bids.add(101, 3);
    bids.add(102, 4);
    EXPECT_EQ(bids.best(), (Level{102, 6}));
    EXPECT_EQ(bids.depth(), 3u);
}

TYPED_TEST(PriceLevelsTest, AsksBestIsLowestAndEmptyLevelsDisappear) {
    TypeParam asks(Side::Sell);
    asks.add(105, 1);
    asks.add(103, 2);
    asks.remove(103, 2);
    EXPECT_EQ(asks.best(), (Level{105, 1}));
    EXPECT_EQ(asks.volume_at(103), 0);
    EXPECT_EQ(asks.depth(), 1u);
}

TEST(OrderBook, AddCancelFillMaintainsLevels) {
    OrderBook book;
    EXPECT_EQ(book.apply(ev(Action::Add, Side::Buy, 1, 100, 10)).outcome, Outcome::Pass);
    EXPECT_EQ(book.apply(ev(Action::Add, Side::Sell, 2, 101, 5)).outcome, Outcome::Pass);
    EXPECT_EQ(book.apply(ev(Action::Fill, Side::Buy, 1, 100, 4)).outcome, Outcome::Pass);
    EXPECT_EQ(book.best_bid(), (Level{100, 6}));
    EXPECT_EQ(book.apply(ev(Action::Cancel, Side::Sell, 2, 101, 5)).outcome, Outcome::Pass);
    EXPECT_FALSE(book.best_ask());
    EXPECT_EQ(book.live_orders(), 1u);
}

TEST(OrderBook, OverfillIsRepairedByClamping) {
    OrderBook book;
    book.apply(ev(Action::Add, Side::Buy, 1, 100, 10));
    const auto r = book.apply(ev(Action::Fill, Side::Buy, 1, 100, 12));
    EXPECT_EQ(r.outcome, Outcome::Repair);
    EXPECT_TRUE(r.flags & kOverfill);
    EXPECT_FALSE(book.best_bid());
    EXPECT_EQ(book.stats().overfills, 1u);
}

TEST(OrderBook, UnknownOrderIsQuarantined) {
    OrderBook book;
    const auto r = book.apply(ev(Action::Cancel, Side::Sell, 42, 100, 1));
    EXPECT_EQ(r.outcome, Outcome::Quarantine);
    EXPECT_TRUE(r.flags & kUnknownOrder);
}

TEST(OrderBook, DuplicateAddAndBadVolumeAreRejected) {
    OrderBook book;
    book.apply(ev(Action::Add, Side::Buy, 1, 100, 1));
    EXPECT_EQ(book.apply(ev(Action::Add, Side::Buy, 1, 100, 1)).outcome, Outcome::Reject);
    EXPECT_EQ(book.apply(ev(Action::Add, Side::Buy, 2, 100, 0)).outcome, Outcome::Reject);
    EXPECT_EQ(book.best_bid(), (Level{100, 1}));
}

TEST(OrderBook, CrossedBookIsFlagged) {
    OrderBook book;
    book.apply(ev(Action::Add, Side::Sell, 1, 100, 1));
    const auto r = book.apply(ev(Action::Add, Side::Buy, 2, 100, 1));
    EXPECT_EQ(r.outcome, Outcome::Pass);
    EXPECT_TRUE(r.flags & kCrossedBook);
    EXPECT_TRUE(book.crossed());
}
