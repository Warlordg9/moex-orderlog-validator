#include "olv/normalize.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace olv {
namespace {

bool all_digits(std::string_view s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); });
}

std::int64_t to_int(std::string_view digits) {
    std::int64_t v = 0;
    for (char c : digits) v = v * 10 + (c - '0');
    return v;
}

}  // namespace

std::optional<std::int64_t> parse_fixed(std::string_view s) {
    const auto dot = s.find('.');
    const std::string_view int_part = s.substr(0, dot);
    const std::string_view frac_part = dot == std::string_view::npos ? std::string_view{} : s.substr(dot + 1);

    // 10 integer digits * 1e8 still fits into int64.
    if (!all_digits(int_part) || int_part.size() > 10) return std::nullopt;
    if (dot != std::string_view::npos && !all_digits(frac_part)) return std::nullopt;
    if (frac_part.size() > 8) return std::nullopt;

    std::int64_t frac = to_int(frac_part);
    for (std::size_t i = frac_part.size(); i < 8; ++i) frac *= 10;
    return to_int(int_part) * kFixedScale + frac;
}

std::optional<std::int64_t> price_to_ticks(std::string_view price, std::int64_t step_fixed) {
    if (step_fixed <= 0) return std::nullopt;
    const auto p = parse_fixed(price);
    if (!p || *p % step_fixed != 0) return std::nullopt;
    return *p / step_fixed;
}

std::optional<std::int64_t> time_to_ns(std::string_view t) {
    if (!all_digits(t)) return std::nullopt;

    std::size_t width = 0;
    std::int64_t frac_to_ns = 0;
    if (t.size() == 8 || t.size() == 9) {
        width = 9;  // HHMMSSmmm
        frac_to_ns = 1'000'000;
    } else if (t.size() == 11 || t.size() == 12) {
        width = 12;  // HHMMSSuuuuuu
        frac_to_ns = 1'000;
    } else {
        return std::nullopt;
    }

    const std::string padded = std::string(width - t.size(), '0') + std::string(t);
    const std::string_view v = padded;
    const auto hh = to_int(v.substr(0, 2));
    const auto mm = to_int(v.substr(2, 2));
    const auto ss = to_int(v.substr(4, 2));
    const auto frac = to_int(v.substr(6));
    if (hh > 23 || mm > 59 || ss > 59) return std::nullopt;

    return ((hh * 60 + mm) * 60 + ss) * 1'000'000'000LL + frac * frac_to_ns;
}

}  // namespace olv
