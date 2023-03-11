#pragma once
#include <queue>

template <typename T> class FreshQueue {
public:
  FreshQueue() {}
  virtual ~FreshQueue() {}
  std::size_t size() const { return m_queue.size(); }

private:
  std::queue<T> m_queue;
};
