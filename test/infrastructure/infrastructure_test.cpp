#include "infrastructure/infrastructure.h"
#include "gtest/gtest.h"
#include <boost/lockfree/queue.hpp>

// Tests for ThreadSafeFreshQueue

TEST(ThreadSafeFreshQueueOfInts, initiallyEmptyPop) {
  ThreadSafeFreshQueue<int> freshQueue{};
  ASSERT_THROW(freshQueue.pop(), EmptyQueue);
}

TEST(ThreadSafeFreshQueueOfInts, initiallyEmptySize) {
  ThreadSafeFreshQueue<int> freshQueue{};
  ASSERT_EQ(freshQueue.size(), 0);
}

TEST(ThreadSafeFreshQueueOfInts, onePushSize) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  ASSERT_EQ(freshQueue.size(), 1);
}

TEST(ThreadSafeFreshQueueOfInts, manyPushSize) {
  using namespace std::views;
  ThreadSafeFreshQueue<int> freshQueue{};
  for (auto &&i : iota(0, 10)) {
    freshQueue.push(i);
  }
  ASSERT_EQ(freshQueue.size(), 10);
}

TEST(ThreadSafeFreshQueueOfInts, initiallyEmptyEmpty) {
  ThreadSafeFreshQueue<int> freshQueue{};
  ASSERT_TRUE(freshQueue.empty());
}

TEST(ThreadSafeFreshQueueOfInts, onePushEmpty) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  ASSERT_FALSE(freshQueue.empty());
}

TEST(ThreadSafeFreshQueueOfInts, onePushAndPopByValueEmpty) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  int value{};
  freshQueue.pop(value);
  ASSERT_TRUE(freshQueue.empty());
}

TEST(ThreadSafeFreshQueueOfInts, onePushAndPopByPointerEmpty) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  auto result{freshQueue.pop()};
  ASSERT_TRUE(freshQueue.empty());
}

TEST(ThreadSafeFreshQueueOfInts, manyPushAndPopByValueSize) {
  using namespace std::views;
  ThreadSafeFreshQueue<int> freshQueue{};
  for (auto &&i : iota(0, 10)) {
    freshQueue.push(i);
  }
  int value{};
  for (auto &&i : iota(0, 5)) {
    freshQueue.pop(value);
  }
  ASSERT_EQ(freshQueue.size(), 5);
}

TEST(ThreadSafeFreshQueueOfInts, manyPushAndPopByPointerSize) {
  using namespace std::views;
  ThreadSafeFreshQueue<int> freshQueue{};
  for (auto &&i : iota(0, 10)) {
    freshQueue.push(i);
  }
  for (auto &&i : iota(0, 5)) {
    auto result{freshQueue.pop()};
  }
  ASSERT_EQ(freshQueue.size(), 5);
}

TEST(ThreadSafeFreshQueueOfInts, initiallyEmptyTryPopByValue) {
  ThreadSafeFreshQueue<int> freshQueue{};
  int value{};
  ASSERT_FALSE(freshQueue.tryPop(value));
}

TEST(ThreadSafeFreshQueueOfInts, initiallyEmptyTryPopByPointer) {
  ThreadSafeFreshQueue<int> freshQueue{};
  ASSERT_EQ(freshQueue.tryPop(), nullptr);
}

TEST(ThreadSafeFreshQueueOfInts, pushAndTryPopByValue) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  int value{};
  ASSERT_TRUE(freshQueue.tryPop(value));
  ASSERT_EQ(value, 42);
}

TEST(ThreadSafeFreshQueueOfInts, pushAndTryPopByPointer) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  auto result{freshQueue.tryPop()};
  ASSERT_EQ(*result, 42);
}

TEST(ThreadSafeFreshQueueOfInts, pushAndWaitAndPopByValue) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  int value{};
  freshQueue.waitAndPop(value);
  ASSERT_EQ(value, 42);
}

TEST(ThreadSafeFreshQueueOfInts, pushAndWaitAndPopByPointer) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  auto result{freshQueue.waitAndPop()};
  ASSERT_EQ(*result, 42);
}

TEST(ThreadSafeFreshQueueOfInts, waitAndPopByValueThenPush) {
  ThreadSafeFreshQueue<int> freshQueue{};
  int value{};
  std::thread popThread{[&] { freshQueue.waitAndPop(value); }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    std::this_thread::sleep_for(10ms);
    freshQueue.push(42);
  }};
  popThread.join();
  pushThread.join();
  ASSERT_EQ(value, 42);
}

TEST(ThreadSafeFreshQueueOfInts, waitAndPopByPointerThenPush) {
  ThreadSafeFreshQueue<int> freshQueue{};
  std::shared_ptr<int> result{};
  std::thread popThread{[&] { result = freshQueue.waitAndPop(); }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    std::this_thread::sleep_for(10ms);
    freshQueue.push(42);
  }};
  popThread.join();
  pushThread.join();
  ASSERT_EQ(*result, 42);
}

TEST(ThreadSafeFreshQueueOfInts, manyWaitAndPopByValueThenPush) {
  ThreadSafeFreshQueue<int> freshQueue{};
  int value{};
  std::thread popThread{[&] {
    using namespace std::views;
    for (auto &&i : iota(0, 10)) {
      freshQueue.waitAndPop(value);
      ASSERT_EQ(value, i);
    }
  }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    using namespace std::views;
    std::this_thread::sleep_for(10ms);
    for (auto &&i : iota(0, 10)) {
      freshQueue.push(i);
    }
  }};
  popThread.join();
  pushThread.join();
}

TEST(ThreadSafeFreshQueueOfInts, manyWaitAndPopByPointerThenPush) {
  ThreadSafeFreshQueue<int> freshQueue{};
  std::shared_ptr<int> result{};
  std::thread popThread{[&] {
    using namespace std::views;
    for (auto &&i : iota(0, 10)) {
      result = freshQueue.waitAndPop();
      ASSERT_EQ(*result, i);
    }
  }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    using namespace std::views;
    std::this_thread::sleep_for(10ms);
    for (auto &&i : iota(0, 10)) {
      freshQueue.push(i);
    }
  }};
  popThread.join();
  pushThread.join();
}

// Tests for ConcurrentFreshQueue

TEST(ConcurrentFreshQueueOfInts, initiallyEmptyEmpty) {
  ConcurrentFreshQueue<int> freshQueue{};
  ASSERT_TRUE(freshQueue.empty());
}

TEST(ConcurrentFreshQueueOfInts, onePushEmpty) {
  ConcurrentFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  ASSERT_FALSE(freshQueue.empty());
}

TEST(ConcurrentFreshQueueOfInts, manyPushEmpty) {
  using namespace std::views;
  ConcurrentFreshQueue<int> freshQueue{};
  for (auto &&i : iota(0, 10)) {
    freshQueue.push(i);
  }
  ASSERT_FALSE(freshQueue.empty());
}

TEST(ConcurrentFreshQueueOfInts, initiallyEmptyTryPopByValue) {
  ConcurrentFreshQueue<int> freshQueue{};
  int value{};
  ASSERT_FALSE(freshQueue.tryPop(value));
}

TEST(ConcurrentFreshQueueOfInts, initiallyEmptyTryPopByPointer) {
  ConcurrentFreshQueue<int> freshQueue{};
  ASSERT_EQ(freshQueue.tryPop(), nullptr);
}

TEST(ConcurrentFreshQueueOfInts, pushAndTryPopByValue) {
  ConcurrentFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  int value{};
  ASSERT_TRUE(freshQueue.tryPop(value));
  ASSERT_EQ(value, 42);
}

TEST(ConcurrentFreshQueueOfInts, pushAndTryPopByPointer) {
  ConcurrentFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  auto result{freshQueue.tryPop()};
  ASSERT_EQ(*result, 42);
}

TEST(ConcurrentFreshQueueOfInts, pushAndWaitAndPopByValue) {
  ConcurrentFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  int value{};
  freshQueue.waitAndPop(value);
  ASSERT_EQ(value, 42);
}

TEST(ConcurrentFreshQueueOfInts, pushAndWaitAndPopByPointer) {
  ConcurrentFreshQueue<int> freshQueue{};
  freshQueue.push(42);
  auto result{freshQueue.waitAndPop()};
  ASSERT_EQ(*result, 42);
}

TEST(ConcurrentFreshQueueOfInts, waitAndPopByValueThenPush) {
  ConcurrentFreshQueue<int> freshQueue{};
  int value{};
  std::thread popThread{[&] { freshQueue.waitAndPop(value); }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    std::this_thread::sleep_for(10ms);
    freshQueue.push(42);
  }};
  popThread.join();
  pushThread.join();
  ASSERT_EQ(value, 42);
}

TEST(ConcurrentFreshQueueOfInts, waitAndPopByPointerThenPush) {
  ConcurrentFreshQueue<int> freshQueue{};
  std::shared_ptr<int> result{};
  std::thread popThread{[&] { result = freshQueue.waitAndPop(); }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    std::this_thread::sleep_for(10ms);
    freshQueue.push(42);
  }};
  popThread.join();
  pushThread.join();
  ASSERT_EQ(*result, 42);
}

TEST(ConcurrentFreshQueueOfInts, manyWaitAndPopByValueThenPush) {
  ConcurrentFreshQueue<int> freshQueue{};
  int value{};
  std::thread popThread{[&] {
    using namespace std::views;
    for (auto &&i : iota(0, 10)) {
      freshQueue.waitAndPop(value);
      ASSERT_EQ(value, i);
    }
  }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    using namespace std::views;
    std::this_thread::sleep_for(10ms);
    for (auto &&i : iota(0, 10)) {
      freshQueue.push(i);
    }
  }};
  popThread.join();
  pushThread.join();
}

TEST(ConcurrentFreshQueueOfInts, manyWaitAndPopByPointerThenPush) {
  ConcurrentFreshQueue<int> freshQueue{};
  std::shared_ptr<int> result{};
  std::thread popThread{[&] {
    using namespace std::views;
    for (auto &&i : iota(0, 10)) {
      result = freshQueue.waitAndPop();
      ASSERT_EQ(*result, i);
    }
  }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    using namespace std::views;
    std::this_thread::sleep_for(10ms);
    for (auto &&i : iota(0, 10)) {
      freshQueue.push(i);
    }
  }};
  popThread.join();
  pushThread.join();
}

// Tests for Boost::lockfree::queue

TEST(LockFreeFreshQueueOfInts, initiallyEmptyEmpty) {
  boost::lockfree::queue<int> freshQueue{10};
  ASSERT_TRUE(freshQueue.empty());
}

TEST(LockFreeFreshQueueOfInts, onePushEmpty) {
  boost::lockfree::queue<int> freshQueue{10};
  freshQueue.push(42);
  ASSERT_FALSE(freshQueue.empty());
}

TEST(LockFreeFreshQueueOfInts, manyPushEmpty) {
  using namespace std::views;
  boost::lockfree::queue<int> freshQueue{10};
  for (auto &&i : iota(0, 100)) {
    freshQueue.push(i);
  }
  ASSERT_FALSE(freshQueue.empty());
}

TEST(LockFreeFreshQueueOfInts, initiallyEmptyPop) {
  boost::lockfree::queue<int> freshQueue{10};
  int value{};
  ASSERT_FALSE(freshQueue.pop(value));
}

TEST(LockFreeFreshQueueOfInts, pushAndPop) {
  boost::lockfree::queue<int> freshQueue{10};
  freshQueue.push(42);
  int value{};
  ASSERT_TRUE(freshQueue.pop(value));
  ASSERT_EQ(value, 42);
}

TEST(LockFreeFreshQueueOfInts, manyWaitAndPopThenPush) {
  boost::lockfree::queue<int> freshQueue{10};
  int value{};
  std::thread popThread{[&] {
    using namespace std::views;
    for (auto &&i : iota(0, 1'000)) {
      while (!freshQueue.pop(value))
        ;
      std::cout << value << std::endl;
      ASSERT_EQ(value, i);
    }
  }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    using namespace std::views;
    for (auto &&i : iota(0, 1'000)) {
      freshQueue.push(i);
    }
  }};
  popThread.join();
  pushThread.join();
}
