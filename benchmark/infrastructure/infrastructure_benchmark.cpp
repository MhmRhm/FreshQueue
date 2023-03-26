#include "benchmark/benchmark.h"
#include "infrastructure/infrastructure.h"

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

template <typename T>
class ThreadSafeFreshQueueMultiThreadFixture : public benchmark::Fixture {
protected:
  ThreadSafeFreshQueue<T> m_freshQueue{};
};

BENCHMARK_TEMPLATE_DEFINE_F(ThreadSafeFreshQueueMultiThreadFixture,
                            ManySeparatedPushesAndPopsByValue, int)
(benchmark::State &state) {
  bool isPushingThread{state.thread_index() % 2 == 0};
  if (isPushingThread) {
    for (auto _ : state) {
      for (int i = state.range(0); i--;) {
        m_freshQueue.push(420);
      }
    }
    state.counters["pushs"] = state.iterations() * state.range(0);
  } else {
    for (auto _ : state) {
      for (int i = state.range(0); i--;) {
        int value{};
        m_freshQueue.waitAndPop(value);
        benchmark::DoNotOptimize(value);
      }
    }
    state.counters["pops"] = state.iterations() * state.range(0);
  }
  state.SetItemsProcessed(
      static_cast<int64_t>(state.iterations() * state.range(0)));
}
BENCHMARK_REGISTER_F(ThreadSafeFreshQueueMultiThreadFixture,
                     ManySeparatedPushesAndPopsByValue)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 10)
    ->ThreadRange(2, std::thread::hardware_concurrency());
