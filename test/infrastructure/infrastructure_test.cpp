#include "infrastructure/infrastructure.h"
#include "gtest/gtest.h"

TEST(FreshQueueOfInts, initiallyEmptySize) {
  FreshQueueWithMutex<int> freshQueue{};
  ASSERT_EQ(freshQueue.size(), 0);
}

TEST(FreshQueueOfInts, initiallyEmptyPush) {
  FreshQueueWithMutex<int> freshQueue{};
  freshQueue.push(1);
  ASSERT_EQ(freshQueue.size(), 1);
}

TEST(FreshQueueOfInts, initiallyEmptyPop) {
  FreshQueueWithMutex<int> freshQueue{};
  ASSERT_THROW(freshQueue.pop(), std::exception);
}
