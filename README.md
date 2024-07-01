# The FreshQueue

Demonstrating a Benchmarking Pipeline

CI/CD pipelines with automated testing have been well-established. In this
article, I will make the case for including benchmarks and argue why they
deserve the same level of attention as automated tests, if not more.

When starting a project, we set guidelines for functionality and performance.
Automated tests ensure our code behaves correctly and hopefully handles all
possible situations. They simplify software testing to the point of pressing a
single button. Additionally, they are invaluable in Test-Driven Development
(TDD), where tests are written first to set expectations for code interaction
and scenario coverage.

The benefits of automated testing are well known, but the same argument is not
so obvious for benchmarks. Imagine a customer with a heavy workload complaining
that your product has become slow. Without preexisting benchmarks, you would
have to go through a long list of commits without knowing where to start.
Testing the software in this scenario would be challenging, and even then, you
might not know which baseline to compare your benchmark results against.

The original team members who designed the performance-critical parts of the
software may no longer be around, having been replaced by new team members, who
in turn may be replaced again. Each group can only compare the software's
performance to what they experienced during their early days with the project.

As a result, you may find yourself in a situation where you don't know which
part of the software is causing the slowdown, which commits have introduced the
issue, how to measure its performance, or, even if you manage to do all that,
which baseline to use for comparison.

On the other hand, if you had automated benchmarks in place from day one, even
years later, new team members would have a clear understanding of the software's
performance history and how it has improved or deteriorated over time. They
would be immediately alerted if a commit caused noticeable performance drops.
Most importantly, they would have a baseline for comparison and a target for
maintaining or improving performance.

To that end, in a case study, I will explain the steps you need to follow to
incorporate a benchmarking workflow into your CI/CD pipelines.

There are many parts involved in this case study:
- The code being benchmarked
- The tests and benchmarking code
- The Git server that runs the CI/CD pipeline
- The CI/CD pipeline and workflows
- The actors that build, test, and benchmark the codebase
- The Docker images used for or by the actors
- The tools used for analyzing the benchmark results

All these pieces come together to form our pipeline. We will go through them one
by one and examine the code and configuration files needed for this case study.

## The Code

We will test and benchmark a few implementations of a concurrent queue. For more
detailed information on concurrency and the implementation of these data
structures, refer to
[C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action).
The full version of the code discussed here is available at
[FreshQueue](https://github.com/MhmRhm/FreshQueue).

Our first implementation is a concurrent queue that uses locks to manage
simultaneous pushes and pops from multiple threads.

```c++
template <typename T> class ThreadSafeFreshQueue {
public:

    ...

  void push(T val) {
    const std::lock_guard lock{m_mutex};
    m_queue.push(std::make_shared<T>(std::move(val)));
    m_pushNotification.notify_one();
  }

  std::shared_ptr<T> tryPop() {
    const std::lock_guard lock{m_mutex};
    if (m_queue.empty())
      return {};
    auto result{m_queue.front()};
    m_queue.pop();
    return result;
  }

  std::shared_ptr<T> waitAndPop() {
    std::unique_lock uniqueLock{m_mutex};
    m_pushNotification.wait(uniqueLock, [&] { return !m_queue.empty(); });
    auto result{m_queue.front()};
    m_queue.pop();
    return result;
  }

  ...

};
```

The second implementation is an enhanced singly linked list with separate locks
on the head and tail to reduce contention on shared data. We expect this data
structure to be more performant than our first one when accessed by multiple
threads.

```c++
template <typename T> class ConcurrentFreshQueue {
private:
  struct Node {
    std::shared_ptr<T> data;
    std::unique_ptr<Node> next;
  };

  ...

  bool tryPopHead(T &value) {
    const std::lock_guard headLock{m_headMutex};
    if (m_head.get() == getTail()) {
      return false;
    }
    value = std::move(*popHead()->data);
    return true;
  }

  ...

public:
  void push(const T &value) {
    auto newTail{std::make_unique<Node>()};
    auto newTailRaw = newTail.get();
    auto newData = std::make_shared<T>(std::move(value));
    {
      std::lock_guard tailLock{m_tailMutex};
      m_tail->data = newData;
      m_tail->next = std::move(newTail);
      m_tail = newTailRaw;
    }
    m_pushNotification.notify_one();
  }

  std::shared_ptr<T> tryPop() {
    auto head{tryPopHead()};
    if (head) {
      return head->data;
    }
    return {};
  }

  ...

};
```

Lastly, we utilize an open-source lock-free queue implementation from the
[Boost library](https://www.boost.org/doc/libs/1_85_0/boost/lockfree/queue.hpp).

## Tests and Benchmarks

For these tasks, we utilize Google libraries available at
[GoogleTest](http://google.github.io/googletest/) and
[Google Benchmark](https://github.com/google/benchmark/blob/main/docs/user_guide.md).
Here's an example of the benchmarking code. It may seem overwhelming at first,
but I'll break it down for you.

```c++
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
```

Here's the main part of the code that we're benchmarking:

```c++
if (isPushingThread) {
  m_queue.push(42);
} else {
  int value{};
  while (!m_queue.pop(value));
}
```

In a pushing thread, we push data to the queue; otherwise, we pop data from it.
Here's a Google Benchmark code snippet that executes the tested code repeatedly
to measure its performance across multiple runs:

```c++
for (auto _ : state) {
  ...
}
```

The `state` variable keeps track of the number of times a piece of code is
executed and the time taken for each execution. We add to its measurement
capabilities using the following:

```c++
state.counters["Pushes"] = benchmark::Counter(
  static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
```

Finally, we instruct the Google Benchmark library on the number of threads to
use when executing our code:

```c++
BENCHMARK_REGISTER_F(BM_LockFreeFreshQueueMultiThreadFixture, PushAndPop)
  ->ThreadRange(2, 1 << 10)
```

The output from this will look like the following:

```c++
.../threads:2            77.4 ns          155 ns     10036644 Pushes=6.45765M/s
.../threads:4             148 ns          593 ns      4214136 Pushes=3.36932M/s
.../threads:8             254 ns         2029 ns      2775024 Pushes=1.96716M/s
.../threads:16            346 ns         3436 ns      1600000 Pushes=1.44463M/s
.../threads:32            349 ns         3475 ns      2341856 Pushes=1.43367M/s
.../threads:64            358 ns         3575 ns      2196800 Pushes=1.39629M/s
.../threads:128           341 ns         3405 ns      2263040 Pushes=1.46528M/s
.../threads:256           258 ns         2579 ns      2560000 Pushes=1.93559M/s
.../threads:512           274 ns         2743 ns      5120000 Pushes=1.82198M/s
.../threads:1024          312 ns         3124 ns     10240000 Pushes=1.60068M/s
```

Now that we have our code and its benchmarking in place, it's time to move on to
the next step, which is setting up our Git server and CI/CD pipelines.

## Git Server

