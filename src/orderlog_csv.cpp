#include "olv/orderlog_csv.hpp"

#include <charconv>
#include <optional>
#include <stdexcept>
#include <string>

#include "olv/normalize.hpp"

namespace olv {
namespace {

constexpr std::size_t kColumns = 10;

std::vector<std::string_view> split(std::string_view line, char sep) {
    std::vector<std::string_view> out;
    std::size_t start = 0;
    while (true) {
        const auto pos = line.find(sep, start);
        out.push_back(line.substr(start, pos - start));
        if (pos == std::string_view::npos) break;
        start = pos + 1;
    }
    return out;
}

std::optional<std::int64_t> parse_int(std::string_view s) {
    std::int64_t v = 0;
    const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), v);
    if (ec != std::errc{} || ptr != s.data() + s.size()) return std::nullopt;
    return v;
}

std::optional<Event> parse_row(const std::vector<std::string_view>& f, std::int64_t step_fixed) {
    if (f.size() != kColumns) return std::nullopt;

    Event e;
    const auto no = parse_int(f[0]);
    const auto time = time_to_ns(f[3]);
    const auto order_no = parse_int(f[4]);
    const auto action = parse_int(f[5]);
    const auto price = price_to_ticks(f[6], step_fixed);
    const auto volume = parse_int(f[7]);
    if (!no || !time || !order_no || !action || !price || !volume) return std::nullopt;
    if (f[1].empty() || (f[2] != "B" && f[2] != "S") || *action < 0 || *action > 2) return std::nullopt;

    e.no = *no;
    e.seccode = std::string(f[1]);
    e.side = f[2] == "B" ? Side::Buy : Side::Sell;
    e.time_ns = *time;
    e.order_no = *order_no;
    e.action = static_cast<Action>(*action);
    e.price_ticks = *price;
    e.volume = *volume;

    if (!f[8].empty()) {
        const auto trade_no = parse_int(f[8]);
        if (!trade_no) return std::nullopt;
        e.trade_no = *trade_no;
    }
    if (!f[9].empty()) {
        const auto trade_price = price_to_ticks(f[9], step_fixed);
        if (!trade_price) return std::nullopt;
        e.trade_price_ticks = *trade_price;
    }
    return e;
}

}  // namespace

ParseResult read_orderlog(std::istream& in, std::string_view price_step) {
    const auto step_fixed = parse_fixed(price_step);
    if (!step_fixed || *step_fixed <= 0) throw std::invalid_argument("invalid price step");

    ParseResult result;
    std::string line;
    std::size_t line_no = 0;
    while (std::getline(in, line)) {
        ++line_no;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.front() == '#' || line.starts_with("NO")) continue;

        if (auto e = parse_row(split(line, ','), *step_fixed)) {
            result.events.push_back(std::move(*e));
        } else {
            result.rejected_lines.push_back(line_no);
        }
    }
    return result;
}

}  // namespace olv
