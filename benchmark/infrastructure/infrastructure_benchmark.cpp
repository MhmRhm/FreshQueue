#include "benchmark/benchmark.h"
#include "infrastructure/infrastructure.h"
#include <boost/lockfree/queue.hpp>

template <typename T> void BM_Queue_PushAndPop(benchmark::State &state) {
  std::queue<T> queue{};
  T value{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;) {
      queue.push(T{});
      value = queue.front();
      queue.pop();
      benchmark::DoNotOptimize(value);
    }
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations() * state.range(0)),
      benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Queue_PushAndPop<int>)->RangeMultiplier(2)->Range(1 << 0, 1 << 5);

template <typename T>
void BM_QueueOfSharedPointer_PushAndPop(benchmark::State &state) {
  std::queue<std::shared_ptr<T>> queue{};
  std::shared_ptr<T> value{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;) {
      queue.push(std::shared_ptr<T>{});
      value = queue.front();
      queue.pop();
      benchmark::DoNotOptimize(value);
    }
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations() * state.range(0)),
      benchmark::Counter::kIsRate);
}
BENCHMARK(BM_QueueOfSharedPointer_PushAndPop<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 5);

template <typename T>
void BM_QueueOfSharedPointer_PushAndPopWithLock(benchmark::State &state) {
  std::queue<std::shared_ptr<T>> queue{};
  std::shared_ptr<T> value{};
  std::mutex mutex;
  for (auto _ : state) {
    for (int i = state.range(0); i--;) {
      {
        std::lock_guard lock{mutex};
        queue.push(std::shared_ptr<T>{});
      }
      {
        std::lock_guard lock{mutex};
        value = queue.front();
        queue.pop();
      }
      benchmark::DoNotOptimize(value);
    }
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations() * state.range(0)),
      benchmark::Counter::kIsRate);
}
BENCHMARK(BM_QueueOfSharedPointer_PushAndPopWithLock<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 5);

template <typename T>
void BM_ThreadSafeFreshQueue_PushAndPop(benchmark::State &state) {
  ThreadSafeFreshQueue<T> queue{};
  T value{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;) {
      queue.push(T{});
      queue.waitAndPop(value);
      benchmark::DoNotOptimize(value);
    }
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations() * state.range(0)),
      benchmark::Counter::kIsRate);
}
BENCHMARK(BM_ThreadSafeFreshQueue_PushAndPop<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 5);

template <typename T>
void BM_ConcurrentFreshQueue_PushAndPop(benchmark::State &state) {
  ConcurrentFreshQueue<T> queue{};
  T value{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;) {
      queue.push(T{});
      queue.waitAndPop(value);
      benchmark::DoNotOptimize(value);
    }
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations() * state.range(0)),
      benchmark::Counter::kIsRate);
}
BENCHMARK(BM_ConcurrentFreshQueue_PushAndPop<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 5);

template <typename T>
void BM_LockFreeFreshQueue_PushAndPop(benchmark::State &state) {
  boost::lockfree::queue<int> queue{10};
  T value{};
  for (auto _ : state) {
    for (int i = state.range(0); i--;) {
      queue.push(T{});
      queue.pop(value);
      benchmark::DoNotOptimize(value);
    }
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations() * state.range(0)),
      benchmark::Counter::kIsRate);
}
BENCHMARK(BM_LockFreeFreshQueue_PushAndPop<int>)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 5);

template <typename T>
class BM_ThreadSafeFreshQueueMultiThreadFixture : public benchmark::Fixture {
protected:
  ThreadSafeFreshQueue<T> m_queue{};
};

BENCHMARK_TEMPLATE_DEFINE_F(BM_ThreadSafeFreshQueueMultiThreadFixture,
                            PushAndPop, int)
(benchmark::State &state) {
  bool isPushingThread{state.thread_index() % 2 == 0};
  if (isPushingThread) {
    for (auto _ : state) {
      for (int i = state.range(0); i--;) {
        m_queue.push(i);
      }
    }
    state.counters["Pushes"] = benchmark::Counter(
        static_cast<int64_t>(state.iterations() * state.range(0)),
        benchmark::Counter::kIsRate);
  } else {
    int value{};
    for (auto _ : state) {
      for (int i = state.range(0); i--;) {
        m_queue.waitAndPop(value);
        benchmark::DoNotOptimize(value);
      }
    }
  }
}
BENCHMARK_REGISTER_F(BM_ThreadSafeFreshQueueMultiThreadFixture, PushAndPop)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 5)
    ->ThreadRange(2, 1 << 10)
    ->MeasureProcessCPUTime()
    ->UseRealTime();

template <typename T>
class BM_ConcurrentFreshQueueMultiThreadFixture : public benchmark::Fixture {
protected:
  ConcurrentFreshQueue<T> m_queue{};
};

BENCHMARK_TEMPLATE_DEFINE_F(BM_ConcurrentFreshQueueMultiThreadFixture,
                            PushAndPop, int)
(benchmark::State &state) {
  bool isPushingThread{state.thread_index() % 2 == 0};
  if (isPushingThread) {
    for (auto _ : state) {
      for (int i = state.range(0); i--;) {
        m_queue.push(i);
      }
    }
    state.counters["Pushes"] = benchmark::Counter(
        static_cast<int64_t>(state.iterations() * state.range(0)),
        benchmark::Counter::kIsRate);
  } else {
    int value{};
    for (auto _ : state) {
      for (int i = state.range(0); i--;) {
        m_queue.waitAndPop(value);
        benchmark::DoNotOptimize(value);
      }
    }
  }
}
BENCHMARK_REGISTER_F(BM_ConcurrentFreshQueueMultiThreadFixture, PushAndPop)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 5)
    ->ThreadRange(2, 1 << 10)
    ->MeasureProcessCPUTime()
    ->UseRealTime();

template <typename T>
class BM_LockFreeFreshQueueMultiThreadFixture : public benchmark::Fixture {
protected:
  boost::lockfree::queue<int> m_queue{10};
};

BENCHMARK_TEMPLATE_DEFINE_F(BM_LockFreeFreshQueueMultiThreadFixture, PushAndPop,
                            int)
(benchmark::State &state) {
  bool isPushingThread{state.thread_index() % 2 == 0};
  if (isPushingThread) {
    for (auto _ : state) {
      for (int i = state.range(0); i--;) {
        m_queue.push(i);
      }
    }
    state.counters["Pushes"] = benchmark::Counter(
        static_cast<int64_t>(state.iterations() * state.range(0)),
        benchmark::Counter::kIsRate);
  } else {
    int value{};
    for (auto _ : state) {
      for (int i = state.range(0); i--;) {
        while (!m_queue.pop(value))
          ;
        benchmark::DoNotOptimize(value);
      }
    }
  }
}
BENCHMARK_REGISTER_F(BM_LockFreeFreshQueueMultiThreadFixture, PushAndPop)
    ->RangeMultiplier(2)
    ->Range(1 << 0, 1 << 5)
    ->ThreadRange(2, 1 << 10)
    ->MeasureProcessCPUTime()
    ->UseRealTime();
