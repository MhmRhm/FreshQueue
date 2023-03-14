#include "benchmark/benchmark.h"
#include "infrastructure/infrastructure.h"
#include "random"
#include "vector"

template <typename T>
void BM_FreshQueueWithMutex_PushAndPopSingleThread(benchmark::State &state) {
  FreshQueueWithMutex<T> freashQueue{};
  for (auto _ : state) {
    freashQueue.push(T{});
    freashQueue.pop();
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}
BENCHMARK(BM_FreshQueueWithMutex_PushAndPopSingleThread<int>);

template <typename T>
void BM_FreshQueueWithMutex_ManyPushsThenManyPopsSingleThread(
    benchmark::State &state) {
  FreshQueueWithMutex<T> freashQueue{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;)
      freashQueue.push(T{});
    for (int i = state.range(0); i--;)
      freashQueue.pop();
  }
  state.SetItemsProcessed(
      static_cast<int64_t>(state.iterations() * state.range(0)));
}
BENCHMARK(BM_FreshQueueWithMutex_ManyPushsThenManyPopsSingleThread<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 10);

template <typename T>
void BM_FreshQueueWithMutex_ManyPushsAndPopsSingleThread(
    benchmark::State &state) {
  FreshQueueWithMutex<T> freashQueue{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;) {
      freashQueue.push(T{});
      freashQueue.pop();
    }
  }
  state.SetItemsProcessed(
      static_cast<int64_t>(state.iterations() * state.range(0)));
}
BENCHMARK(BM_FreshQueueWithMutex_ManyPushsAndPopsSingleThread<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 10);
