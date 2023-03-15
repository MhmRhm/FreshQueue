#include "algorithm"
#include "infrastructure/infrastructure.h"
#include "ranges"
#include "gtest/gtest.h"

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
    auto value{freshQueue.pop()};
  }
  ASSERT_EQ(freshQueue.size(), 5);
}

TEST(ThreadSafeFreshQueueOfInts, initiallyEmptyPop) {
  ThreadSafeFreshQueue<int> freshQueue{};
  ASSERT_THROW(freshQueue.pop(), empty_queue);
}
