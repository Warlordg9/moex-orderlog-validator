#include "olv/price_levels.hpp"

#include <algorithm>

namespace olv {

// ---- MapLevels ----

void MapLevels::add(std::int64_t price_ticks, std::int64_t volume) {
    levels_[price_ticks] += volume;
}

void MapLevels::remove(std::int64_t price_ticks, std::int64_t volume) {
    const auto it = levels_.find(price_ticks);
    if (it == levels_.end()) return;
    it->second -= volume;
    if (it->second <= 0) levels_.erase(it);
}

std::optional<Level> MapLevels::best() const {
    if (levels_.empty()) return std::nullopt;
    const auto& [price, volume] = side_ == Side::Buy ? *levels_.rbegin() : *levels_.begin();
    return Level{price, volume};
}

std::int64_t MapLevels::volume_at(std::int64_t price_ticks) const {
    const auto it = levels_.find(price_ticks);
    return it == levels_.end() ? 0 : it->second;
}

// ---- VectorLevels ----

std::vector<Level>::iterator VectorLevels::find_slot(std::int64_t price_ticks) {
    if (side_ == Side::Buy) {
        return std::lower_bound(levels_.begin(), levels_.end(), price_ticks,
                                [](const Level& l, std::int64_t p) { return l.price_ticks < p; });
    }
    return std::lower_bound(levels_.begin(), levels_.end(), price_ticks,
                            [](const Level& l, std::int64_t p) { return l.price_ticks > p; });
}

void VectorLevels::add(std::int64_t price_ticks, std::int64_t volume) {
    const auto it = find_slot(price_ticks);
    if (it != levels_.end() && it->price_ticks == price_ticks) {
        it->volume += volume;
    } else {
        levels_.insert(it, Level{price_ticks, volume});
    }
}

void VectorLevels::remove(std::int64_t price_ticks, std::int64_t volume) {
    const auto it = find_slot(price_ticks);
    if (it == levels_.end() || it->price_ticks != price_ticks) return;
    it->volume -= volume;
    if (it->volume <= 0) levels_.erase(it);
}

std::optional<Level> VectorLevels::best() const {
    if (levels_.empty()) return std::nullopt;
    return levels_.back();
}

std::int64_t VectorLevels::volume_at(std::int64_t price_ticks) const {
    const auto it = const_cast<VectorLevels*>(this)->find_slot(price_ticks);
    return it != levels_.end() && it->price_ticks == price_ticks ? it->volume : 0;
}

}  // namespace olv
