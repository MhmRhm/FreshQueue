#include "benchmark/benchmark.h"
#include "infrastructure/infrastructure.h"
#include "random"
#include "vector"

template <typename T>
void BM_ThreadSafeFreshQueue_PushAndPopByValueSingleThread(
    benchmark::State &state) {
  ThreadSafeFreshQueue<T> freshQueue{};
  for (auto _ : state) {
    freshQueue.push(T{});
    T value{};
    freshQueue.pop(value);
    benchmark::DoNotOptimize(value);
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}
BENCHMARK(BM_ThreadSafeFreshQueue_PushAndPopByValueSingleThread<int>);

template <typename T>
void BM_ThreadSafeFreshQueue_PushAndPopByPointerSingleThread(
    benchmark::State &state) {
  ThreadSafeFreshQueue<T> freshQueue{};
  for (auto _ : state) {
    freshQueue.push(T{});
    auto value{freshQueue.pop()};
    benchmark::DoNotOptimize(value);
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}
BENCHMARK(BM_ThreadSafeFreshQueue_PushAndPopByPointerSingleThread<int>);

template <typename T>
void BM_ThreadSafeFreshQueue_ManyPushesThenManyPopsByValueSingleThread(
    benchmark::State &state) {
  ThreadSafeFreshQueue<T> freshQueue{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;)
      freshQueue.push(T{});
    for (int i = state.range(0); i--;) {
      T value{};
      freshQueue.pop(value);
      benchmark::DoNotOptimize(value);
    }
  }
  state.SetItemsProcessed(
      static_cast<int64_t>(state.iterations() * state.range(0)));
}
BENCHMARK(
    BM_ThreadSafeFreshQueue_ManyPushesThenManyPopsByValueSingleThread<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 10);

template <typename T>
void BM_ThreadSafeFreshQueue_ManyPushesThenManyPopsByPointerSingleThread(
    benchmark::State &state) {
  ThreadSafeFreshQueue<T> freshQueue{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;)
      freshQueue.push(T{});
    for (int i = state.range(0); i--;) {
      auto value{freshQueue.pop()};
      benchmark::DoNotOptimize(value);
    }
  }
  state.SetItemsProcessed(
      static_cast<int64_t>(state.iterations() * state.range(0)));
}
BENCHMARK(
    BM_ThreadSafeFreshQueue_ManyPushesThenManyPopsByPointerSingleThread<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 10);

template <typename T>
void BM_ThreadSafeFreshQueue_ManyPushesAndPopsByValueSingleThread(
    benchmark::State &state) {
  ThreadSafeFreshQueue<T> freshQueue{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;) {
      freshQueue.push(T{});
      T value{};
      freshQueue.pop(value);
      benchmark::DoNotOptimize(value);
    }
  }
  state.SetItemsProcessed(
      static_cast<int64_t>(state.iterations() * state.range(0)));
}
BENCHMARK(BM_ThreadSafeFreshQueue_ManyPushesAndPopsByValueSingleThread<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 10);

template <typename T>
void BM_ThreadSafeFreshQueue_ManyPushesAndPopsByPointerSingleThread(
    benchmark::State &state) {
  ThreadSafeFreshQueue<T> freshQueue{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;) {
      freshQueue.push(T{});
      auto value{freshQueue.pop()};
      benchmark::DoNotOptimize(value);
    }
  }
  state.SetItemsProcessed(
      static_cast<int64_t>(state.iterations() * state.range(0)));
}
BENCHMARK(BM_ThreadSafeFreshQueue_ManyPushesAndPopsByPointerSingleThread<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 10);
