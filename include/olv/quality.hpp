/// @file quality.hpp
/// @brief Rule outcomes and per-event quality flags.
#pragma once

#include <cstdint>
#include <string_view>

namespace olv {

/// Result of the rule catalogue for one event. Nothing is dropped silently:
/// every event ends in exactly one of these four states.
enum class Outcome : std::uint8_t {
    Pass,        ///< Event is consistent and applied as is.
    Repair,      ///< Event applied after a deterministic, logged correction.
    Quarantine,  ///< Event cannot be applied but is kept for inspection.
    Reject,      ///< Event is invalid and not applied.
};

/// Bit flags attached to every output event.
enum QualityFlag : std::uint32_t {
    kNone = 0,
    kCrossedBook = 1u << 0,     ///< Best bid >= best ask after applying the event.
    kOverfill = 1u << 1,        ///< Fill volume exceeded the order's remaining volume.
    kUnknownOrder = 1u << 2,    ///< Cancel/fill for an order that is not in the book.
    kDuplicateOrder = 1u << 3,  ///< Add for an order number that is already live.
    kInvalidVolume = 1u << 4,   ///< Volume <= 0.
    kVolumeClamped = 1u << 5,   ///< Cancel volume exceeded remaining and was clamped.
};

/// Outcome plus accumulated flags for one event.
struct RuleResult {
    Outcome outcome = Outcome::Pass;
    std::uint32_t flags = kNone;
};

/// Human-readable outcome name.
constexpr std::string_view to_string(Outcome o) {
    switch (o) {
        case Outcome::Pass: return "pass";
        case Outcome::Repair: return "repair";
        case Outcome::Quarantine: return "quarantine";
        case Outcome::Reject: return "reject";
    }
    return "unknown";
}

}  // namespace olv
