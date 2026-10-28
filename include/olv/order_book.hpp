/// @file order_book.hpp
/// @brief Order book engine: OrderLog events -> aggregated price levels + rule checks.
#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "olv/price_levels.hpp"
#include "olv/quality.hpp"
#include "olv/types.hpp"

namespace olv {

/// Counters accumulated while replaying one instrument.
struct BookStats {
    std::uint64_t events = 0;
    std::uint64_t passed = 0;
    std::uint64_t repaired = 0;
    std::uint64_t quarantined = 0;
    std::uint64_t rejected = 0;
    std::uint64_t crossed = 0;    ///< Events after which the book was crossed.
    std::uint64_t overfills = 0;  ///< Fills larger than the remaining order volume.

    friend bool operator==(const BookStats&, const BookStats&) = default;
};

/// Order book for a single instrument.
///
/// Rules applied to each event:
/// - volume <= 0 -> Reject (kInvalidVolume)
/// - add of a live order number -> Reject (kDuplicateOrder)
/// - cancel/fill of an unknown order -> Quarantine (kUnknownOrder)
/// - fill larger than remaining -> Repair: clamp to remaining (kOverfill)
/// - cancel larger than remaining -> Repair: clamp to remaining (kVolumeClamped)
/// - book crossed after the event -> flag kCrossedBook (outcome unchanged)
class OrderBook {
public:
    /// Applies one event and returns its outcome and flags.
    RuleResult apply(const Event& e);

    std::optional<Level> best_bid() const { return bids_.best(); }
    std::optional<Level> best_ask() const { return asks_.best(); }
    /// True if best bid >= best ask.
    bool crossed() const;
    /// Number of live orders.
    std::size_t live_orders() const { return orders_.size(); }
    const BookStats& stats() const { return stats_; }

private:
    struct LiveOrder {
        Side side;
        std::int64_t price_ticks;
        std::int64_t remaining;
    };

    RuleResult apply_rules(const Event& e);
    MapLevels& levels(Side s) { return s == Side::Buy ? bids_ : asks_; }

    MapLevels bids_{Side::Buy};
    MapLevels asks_{Side::Sell};
    std::unordered_map<std::int64_t, LiveOrder> orders_;
    BookStats stats_;
};

/// Replays events through one OrderBook per instrument.
/// @return per-instrument statistics, ordered by SECCODE.
std::map<std::string, BookStats> replay(const std::vector<Event>& events);

}  // namespace olv
