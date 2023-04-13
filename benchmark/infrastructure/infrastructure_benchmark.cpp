#include "benchmark/benchmark.h"
#include "infrastructure/infrastructure.h"
#include <boost/lockfree/queue.hpp>

template <typename T> void BM_Queue_PushAndPop(benchmark::State &state) {
  std::queue<T> queue{};
  T value{};
  for (auto _ : state) {
    queue.push(T{});
    value = queue.front();
    queue.pop();
    benchmark::DoNotOptimize(value);
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Queue_PushAndPop<int>);

template <typename T>
void BM_QueueOfSharedPointer_PushAndPop(benchmark::State &state) {
  std::queue<std::shared_ptr<T>> queue{};
  std::shared_ptr<T> value{};
  for (auto _ : state) {
    queue.push(std::shared_ptr<T>{});
    value = queue.front();
    queue.pop();
    benchmark::DoNotOptimize(value);
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_QueueOfSharedPointer_PushAndPop<int>);

template <typename T>
void BM_QueueOfSharedPointer_PushAndPopWithLock(benchmark::State &state) {
  std::queue<std::shared_ptr<T>> queue{};
  std::shared_ptr<T> value{};
  std::mutex mutex;
  for (auto _ : state) {
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
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_QueueOfSharedPointer_PushAndPopWithLock<int>);

template <typename T>
void BM_ThreadSafeFreshQueue_PushAndPop(benchmark::State &state) {
  ThreadSafeFreshQueue<T> queue{};
  T value{};
  for (auto _ : state) {
    queue.push(T{});
    queue.waitAndPop(value);
    benchmark::DoNotOptimize(value);
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_ThreadSafeFreshQueue_PushAndPop<int>);

template <typename T>
void BM_ConcurrentFreshQueue_PushAndPop(benchmark::State &state) {
  ConcurrentFreshQueue<T> queue{};
  T value{};
  for (auto _ : state) {
    queue.push(T{});
    queue.waitAndPop(value);
    benchmark::DoNotOptimize(value);
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_ConcurrentFreshQueue_PushAndPop<int>);

template <typename T>
void BM_LockFreeFreshQueue_PushAndPop(benchmark::State &state) {
  boost::lockfree::queue<int> queue{10};
  T value{};
  for (auto _ : state) {
    queue.push(T{});
    queue.pop(value);
    benchmark::DoNotOptimize(value);
  }
  state.counters["Pushes"] = benchmark::Counter(
      static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_LockFreeFreshQueue_PushAndPop<int>);

template <typename T>
class BM_QueueMultiThreadFixture : public benchmark::Fixture {
protected:
  std::queue<std::shared_ptr<T>> m_queue{};
  std::mutex m_mutex;
  std::condition_variable m_pushNotification;
};
BENCHMARK_TEMPLATE_DEFINE_F(BM_QueueMultiThreadFixture, PushAndPop, int)
(benchmark::State &state) {
  bool isPushingThread{state.thread_index() % 2 == 0};
  if (isPushingThread) {
    for (auto _ : state) {
      {
        std::lock_guard lock{m_mutex};
        m_queue.push(std::make_shared<int>(42));
      }
      m_pushNotification.notify_one();
    }
    state.counters["Pushes"] = benchmark::Counter(
        static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
  } else {
    std::shared_ptr<int> value{};
    for (auto _ : state) {
      std::unique_lock lock{m_mutex};
      m_pushNotification.wait(lock, [&] { return !m_queue.empty(); });
      value = m_queue.front();
      m_queue.pop();
      benchmark::DoNotOptimize(value);
    }
  }
}
BENCHMARK_REGISTER_F(BM_QueueMultiThreadFixture, PushAndPop)
    ->ThreadRange(2, 1 << 10)
    ->MeasureProcessCPUTime()
    ->UseRealTime();

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
      m_queue.push(42);
    }
    state.counters["Pushes"] = benchmark::Counter(
        static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
  } else {
    int value{};
    for (auto _ : state) {
      m_queue.waitAndPop(value);
      benchmark::DoNotOptimize(value);
    }
  }
}
BENCHMARK_REGISTER_F(BM_ThreadSafeFreshQueueMultiThreadFixture, PushAndPop)
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
      m_queue.push(42);
    }
    state.counters["Pushes"] = benchmark::Counter(
        static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
  } else {
    int value{};
    for (auto _ : state) {
      m_queue.waitAndPop(value);
      benchmark::DoNotOptimize(value);
    }
  }
}
BENCHMARK_REGISTER_F(BM_ConcurrentFreshQueueMultiThreadFixture, PushAndPop)
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
      m_queue.push(42);
    }
    state.counters["Pushes"] = benchmark::Counter(
        static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
  } else {
    int value{};
    for (auto _ : state) {
      while (!m_queue.pop(value))
        ;
      benchmark::DoNotOptimize(value);
    }
  }
}
BENCHMARK_REGISTER_F(BM_LockFreeFreshQueueMultiThreadFixture, PushAndPop)
    ->ThreadRange(2, 1 << 10)
    ->MeasureProcessCPUTime()
    ->UseRealTime();
