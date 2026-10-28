// Benchmarks for price-level storage (ADR 0001) and full order-book replay.
#include <benchmark/benchmark.h>

#include <random>
#include <vector>

#include "olv/order_book.hpp"
#include "olv/price_levels.hpp"

namespace {

struct Op {
    bool add;
    std::int64_t price;
    std::int64_t volume;
};

// Random walk around a mid price: most updates land near the top of book,
// which is what real OrderLog traffic looks like.
std::vector<Op> make_ops(std::size_t n, int spread_ticks) {
    std::mt19937_64 rng(42);
    std::normal_distribution<double> offset(0.0, spread_ticks / 3.0);
    std::uniform_int_distribution<int> vol(1, 10);
    std::vector<Op> ops;
    ops.reserve(n);
    std::vector<Op> live;
    for (std::size_t i = 0; i < n; ++i) {
        if (live.empty() || i % 2 == 0) {
            const auto price = 100'000 - 1 - static_cast<std::int64_t>(std::abs(offset(rng)));
            const Op op{true, price, vol(rng)};
            ops.push_back(op);
            live.push_back(op);
        } else {
            std::uniform_int_distribution<std::size_t> pick(0, live.size() - 1);
            const auto idx = pick(rng);
            ops.push_back({false, live[idx].price, live[idx].volume});
            live[idx] = live.back();
            live.pop_back();
        }
    }
    return ops;
}

template <typename Levels>
void BM_PriceLevels(benchmark::State& state) {
    const auto ops = make_ops(100'000, static_cast<int>(state.range(0)));
    for (auto _ : state) {
        Levels bids(olv::Side::Buy);
        for (const auto& op : ops) {
            if (op.add) {
                bids.add(op.price, op.volume);
            } else {
                bids.remove(op.price, op.volume);
            }
            benchmark::DoNotOptimize(bids.best());
        }
    }
    state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations() * ops.size()));
}

void BM_OrderBookReplay(benchmark::State& state) {
    const auto ops = make_ops(100'000, 50);
    std::vector<olv::Event> events;
    events.reserve(ops.size());
    std::vector<std::int64_t> live;
    std::int64_t next_order = 1;
    std::mt19937_64 rng(7);
    for (const auto& op : ops) {
        olv::Event e;
        e.seccode = "BENCH";
        e.side = olv::Side::Buy;
        e.price_ticks = op.price;
        e.volume = op.volume;
        if (op.add || live.empty()) {
            e.action = olv::Action::Add;
            e.order_no = next_order;
            live.push_back(next_order++);
        } else {
            std::uniform_int_distribution<std::size_t> pick(0, live.size() - 1);
            const auto idx = pick(rng);
            e.action = olv::Action::Cancel;
            e.order_no = live[idx];
            e.volume = 1'000;  // clamped to remaining: removes the whole order
            live[idx] = live.back();
            live.pop_back();
        }
        events.push_back(e);
    }

    for (auto _ : state) {
        olv::OrderBook book;
        for (const auto& e : events) benchmark::DoNotOptimize(book.apply(e));
    }
    state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations() * events.size()));
}

}  // namespace

BENCHMARK(BM_PriceLevels<olv::MapLevels>)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK(BM_PriceLevels<olv::VectorLevels>)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK(BM_OrderBookReplay);

BENCHMARK_MAIN();
