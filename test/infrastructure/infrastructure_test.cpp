#include "algorithm"
#include "infrastructure/infrastructure.h"
#include "ranges"
#include "gtest/gtest.h"

TEST(FreshQueueWithMutexOfInts, initiallyEmptySize) {
  FreshQueueWithMutex<int> freshQueue{};
  ASSERT_EQ(freshQueue.size(), 0);
}

TEST(FreshQueueWithMutexOfInts, onePushSize) {
  FreshQueueWithMutex<int> freshQueue{};
  freshQueue.push(0);
  ASSERT_EQ(freshQueue.size(), 1);
}

TEST(FreshQueueWithMutexOfInts, manyPushSize) {
  using namespace std::views;
  FreshQueueWithMutex<int> freshQueue{};
  for (auto &&i : iota(0, 10)) {
    freshQueue.push(i);
  }
  ASSERT_EQ(freshQueue.size(), 10);
}

TEST(FreshQueueWithMutexOfInts, manyPushPopSize) {
  using namespace std::views;
  FreshQueueWithMutex<int> freshQueue{};
  for (auto &&i : iota(0, 10)) {
    freshQueue.push(i);
  }
  for (auto &&i : iota(0, 5)) {
    freshQueue.pop();
  }
  ASSERT_EQ(freshQueue.size(), 5);
}

TEST(FreshQueueWithMutexOfInts, initiallyEmptyPop) {
  FreshQueueWithMutex<int> freshQueue{};
  ASSERT_THROW(freshQueue.pop(), std::exception);
}
