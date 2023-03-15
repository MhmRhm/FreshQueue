#include "infrastructure/infrastructure.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <chrono>
#include <ranges>
#include <thread>

TEST(ThreadSafeFreshQueueOfInts, initiallyEmptyPop) {
  ThreadSafeFreshQueue<int> freshQueue{};
  ASSERT_THROW(freshQueue.pop(), empty_queue);
}

TEST(ThreadSafeFreshQueueOfInts, initiallyEmptySize) {
  ThreadSafeFreshQueue<int> freshQueue{};
  ASSERT_EQ(freshQueue.size(), 0);
}

TEST(ThreadSafeFreshQueueOfInts, onePushSize) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(0);
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
  freshQueue.push(0);
  ASSERT_FALSE(freshQueue.empty());
}

TEST(ThreadSafeFreshQueueOfInts, onePushAndPopByValueEmpty) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(0);
  int value{};
  freshQueue.pop(value);
  ASSERT_TRUE(freshQueue.empty());
}

TEST(ThreadSafeFreshQueueOfInts, onePushAndPopByPointerEmpty) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(0);
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
  freshQueue.push(420);
  int value{};
  ASSERT_TRUE(freshQueue.tryPop(value));
  ASSERT_EQ(value, 420);
}

TEST(ThreadSafeFreshQueueOfInts, pushAndTryPopByPointer) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(420);
  auto result{freshQueue.tryPop()};
  ASSERT_EQ(*result, 420);
}

TEST(ThreadSafeFreshQueueOfInts, pushAndWaitAndPopByValue) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(420);
  int value{};
  freshQueue.waitAndPop(value);
  ASSERT_EQ(value, 420);
}

TEST(ThreadSafeFreshQueueOfInts, pushAndWaitAndPopByPointer) {
  ThreadSafeFreshQueue<int> freshQueue{};
  freshQueue.push(420);
  auto result{freshQueue.waitAndPop()};
  ASSERT_EQ(*result, 420);
}

TEST(ThreadSafeFreshQueueOfInts, waitAndPopByValueThenPush) {
  ThreadSafeFreshQueue<int> freshQueue{};
  int value{};
  std::thread popThread{[&] { freshQueue.waitAndPop(value); }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    std::this_thread::sleep_for(1ms);
    freshQueue.push(420);
  }};
  popThread.join();
  pushThread.join();
  ASSERT_EQ(value, 420);
}

TEST(ThreadSafeFreshQueueOfInts, waitAndPopByPointerThenPush) {
  ThreadSafeFreshQueue<int> freshQueue{};
  std::shared_ptr<int> result{};
  std::thread popThread{[&] { result = freshQueue.waitAndPop(); }};
  std::thread pushThread{[&] {
    using namespace std::chrono;
    std::this_thread::sleep_for(1ms);
    freshQueue.push(420);
  }};
  popThread.join();
  pushThread.join();
  ASSERT_EQ(*result, 420);
}
