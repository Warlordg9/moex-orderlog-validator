/// @file price_levels.hpp
/// @brief Aggregated price-level storage for one side of the book.
///
/// Two interchangeable implementations are kept so that the choice can be
/// justified by the benchmark (see docs/adr/0001-price-level-storage.md).
#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "olv/types.hpp"

namespace olv {

/// Aggregated volume at one price.
struct Level {
    std::int64_t price_ticks = 0;
    std::int64_t volume = 0;

    friend bool operator==(const Level&, const Level&) = default;
};

/// Price levels stored in a std::map (O(log n) insert/erase, stable iterators).
class MapLevels {
public:
    explicit MapLevels(Side side) : side_(side) {}

    /// Adds volume at a price, creating the level if needed.
    void add(std::int64_t price_ticks, std::int64_t volume);
    /// Removes volume at a price; the level disappears when it reaches zero.
    void remove(std::int64_t price_ticks, std::int64_t volume);
    /// Best level: highest bid or lowest ask.
    std::optional<Level> best() const;
    /// Volume at a price, 0 if the level does not exist.
    std::int64_t volume_at(std::int64_t price_ticks) const;
    /// Number of non-empty levels.
    std::size_t depth() const { return levels_.size(); }

private:
    Side side_;
    std::map<std::int64_t, std::int64_t> levels_;
};

/// Price levels stored in a sorted std::vector with the best level at the back,
/// so that the most frequent updates (near the top of book) move few elements.
class VectorLevels {
public:
    explicit VectorLevels(Side side) : side_(side) {}

    /// @copydoc MapLevels::add
    void add(std::int64_t price_ticks, std::int64_t volume);
    /// @copydoc MapLevels::remove
    void remove(std::int64_t price_ticks, std::int64_t volume);
    /// @copydoc MapLevels::best
    std::optional<Level> best() const;
    /// @copydoc MapLevels::volume_at
    std::int64_t volume_at(std::int64_t price_ticks) const;
    /// @copydoc MapLevels::depth
    std::size_t depth() const { return levels_.size(); }

private:
    std::vector<Level>::iterator find_slot(std::int64_t price_ticks);

    Side side_;
    std::vector<Level> levels_;  // bids ascending, asks descending: best is back()
};

}  // namespace olv
