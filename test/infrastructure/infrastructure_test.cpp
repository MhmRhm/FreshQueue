#include "infrastructure/infrastructure.h"
#include "gtest/gtest.h"

TEST(FreshQueue, isInitiallyEmpty)
{
    FreshQueue<int> freshQueue{};
    ASSERT_EQ(freshQueue.size(), 0);
}
