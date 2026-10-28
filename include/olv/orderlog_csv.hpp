/// @file orderlog_csv.hpp
/// @brief Reader for MOEX OrderLog CSV files.
#pragma once

#include <cstddef>
#include <istream>
#include <string_view>
#include <vector>

#include "olv/types.hpp"

namespace olv {

/// Parsed file: valid events plus line numbers that failed to parse.
struct ParseResult {
    std::vector<Event> events;
    std::vector<std::size_t> rejected_lines;  ///< 1-based line numbers.
};

/// Reads OrderLog CSV with columns
/// NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE.
/// Empty lines, lines starting with '#' and header lines are skipped.
/// Lines with malformed fields or off-grid prices are reported, not dropped silently.
/// @param price_step decimal price step, e.g. "0.01"
ParseResult read_orderlog(std::istream& in, std::string_view price_step);

}  // namespace olv
