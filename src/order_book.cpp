#include "olv/order_book.hpp"

#include <algorithm>

namespace olv {

bool OrderBook::crossed() const {
    const auto bid = best_bid();
    const auto ask = best_ask();
    return bid && ask && bid->price_ticks >= ask->price_ticks;
}

RuleResult OrderBook::apply_rules(const Event& e) {
    if (e.volume <= 0) return {Outcome::Reject, kInvalidVolume};

    if (e.action == Action::Add) {
        if (orders_.contains(e.order_no)) return {Outcome::Reject, kDuplicateOrder};
        orders_.emplace(e.order_no, LiveOrder{e.side, e.price_ticks, e.volume});
        levels(e.side).add(e.price_ticks, e.volume);
        return {};
    }

    const auto it = orders_.find(e.order_no);
    if (it == orders_.end()) return {Outcome::Quarantine, kUnknownOrder};

    LiveOrder& order = it->second;
    RuleResult result;
    std::int64_t volume = e.volume;
    if (volume > order.remaining) {
        result = {Outcome::Repair, e.action == Action::Fill ? kOverfill : kVolumeClamped};
        volume = order.remaining;
    }

    levels(order.side).remove(order.price_ticks, volume);
    order.remaining -= volume;
    if (order.remaining == 0) orders_.erase(it);
    return result;
}

RuleResult OrderBook::apply(const Event& e) {
    RuleResult r = apply_rules(e);
    if (crossed()) r.flags |= kCrossedBook;

    ++stats_.events;
    switch (r.outcome) {
        case Outcome::Pass: ++stats_.passed; break;
        case Outcome::Repair: ++stats_.repaired; break;
        case Outcome::Quarantine: ++stats_.quarantined; break;
        case Outcome::Reject: ++stats_.rejected; break;
    }
    if (r.flags & kCrossedBook) ++stats_.crossed;
    if (r.flags & kOverfill) ++stats_.overfills;
    return r;
}

std::map<std::string, BookStats> replay(const std::vector<Event>& events) {
    std::unordered_map<std::string, OrderBook> books;
    for (const auto& e : events) books[e.seccode].apply(e);

    std::map<std::string, BookStats> out;
    for (const auto& [code, book] : books) out.emplace(code, book.stats());
    return out;
}

}  // namespace olv
