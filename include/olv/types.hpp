/// @file types.hpp
/// @brief Normalized OrderLog event and basic enums.
#pragma once

#include <cstdint>
#include <string>

namespace olv {

/// Order side.
enum class Side : std::uint8_t { Buy, Sell };

/// OrderLog action code (MOEX: 0 = cancel, 1 = add, 2 = trade).
enum class Action : std::uint8_t { Cancel = 0, Add = 1, Fill = 2 };

/// One normalized OrderLog row: prices in integer ticks, time in nanoseconds.
struct Event {
    std::int64_t no = 0;                 ///< Row number in the source log (NO).
    std::string seccode;                 ///< Instrument code (SECCODE).
    Side side = Side::Buy;               ///< BUYSELL.
    std::int64_t time_ns = 0;            ///< Nanoseconds since midnight, exchange time.
    std::int64_t order_no = 0;           ///< ORDERNO.
    Action action = Action::Add;         ///< ACTION.
    std::int64_t price_ticks = 0;        ///< PRICE / price_step.
    std::int64_t volume = 0;             ///< VOLUME in lots.
    std::int64_t trade_no = 0;           ///< TRADENO, 0 if absent.
    std::int64_t trade_price_ticks = 0;  ///< TRADEPRICE / price_step, 0 if absent.
};

}  // namespace olv
