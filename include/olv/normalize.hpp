/// @file normalize.hpp
/// @brief Exact (floating-point free) normalization of prices and timestamps.
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace olv {

/// Fixed-point scale used for decimal parsing: 1e-8.
inline constexpr std::int64_t kFixedScale = 100'000'000;

/// Parses a non-negative decimal string ("250.10") into fixed-point units of 1e-8.
/// @return std::nullopt on malformed input or more than 8 fractional digits.
std::optional<std::int64_t> parse_fixed(std::string_view s);

/// Converts a decimal price to an integer number of price steps.
/// @param price decimal price as in the source file
/// @param step_fixed price step in 1e-8 units (see parse_fixed)
/// @return std::nullopt if the price is malformed or not on the price grid.
std::optional<std::int64_t> price_to_ticks(std::string_view price, std::int64_t step_fixed);

/// Converts OrderLog TIME to nanoseconds since midnight.
/// Accepts HHMMSSmmm (8-9 digits, milliseconds) and HHMMSSuuuuuu (11-12 digits,
/// microseconds); the leading zero of the hour may be omitted.
/// @return std::nullopt on malformed input or out-of-range fields.
std::optional<std::int64_t> time_to_ns(std::string_view t);

}  // namespace olv
