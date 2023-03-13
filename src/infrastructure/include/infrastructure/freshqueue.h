#pragma once
#include <exception>
#include <mutex>
#include <queue>
#include <vector>

template <typename T> class FreshQueueWithMutex {
public:
  FreshQueueWithMutex() = default;
  FreshQueueWithMutex(const FreshQueueWithMutex &) = delete;
  FreshQueueWithMutex(FreshQueueWithMutex &&) noexcept = delete;
  FreshQueueWithMutex &operator=(const FreshQueueWithMutex &) = delete;
  FreshQueueWithMutex &operator=(FreshQueueWithMutex &&) noexcept = delete;
  virtual ~FreshQueueWithMutex() = default;

  std::size_t size() const {
    const std::lock_guard<std::mutex> lock{m_mutex};
    return m_queue.size();
  }
  void push(const T &val) {
    const std::lock_guard<std::mutex> lock{m_mutex};
    m_queue.push(val);
  }
  T pop() {
    const std::lock_guard<std::mutex> lock{m_mutex};
    if (m_queue.empty()) {
      throw std::runtime_error("pop from empty FreshQueueWithMutex!");
    }
    T val{m_queue.front()};
    m_queue.pop();
    return val;
  }

private:
  std::queue<T> m_queue;
  mutable std::mutex m_mutex;
};
