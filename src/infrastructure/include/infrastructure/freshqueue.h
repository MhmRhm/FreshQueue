#pragma once
#include <exception>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

class empty_queue : std::exception {
  virtual const char *what() const noexcept override {
    return "called pop on empty queue";
  };
};

template <typename T> class ThreadSafeFreshQueue {
public:
  ThreadSafeFreshQueue() = default;
  ThreadSafeFreshQueue(const ThreadSafeFreshQueue &) = delete;
  ThreadSafeFreshQueue(ThreadSafeFreshQueue &&) noexcept = delete;
  ThreadSafeFreshQueue &operator=(const ThreadSafeFreshQueue &) = delete;
  ThreadSafeFreshQueue &operator=(ThreadSafeFreshQueue &&) noexcept = delete;
  virtual ~ThreadSafeFreshQueue() = default;

  std::size_t size() const {
    const std::lock_guard<std::mutex> lock{m_mutex};
    return m_queue.size();
  }
  void push(const T &val) {
    const std::lock_guard<std::mutex> lock{m_mutex};
    m_queue.push(val);
  }
  void pop(T &value) {
    const std::lock_guard<std::mutex> lock{m_mutex};
    if (m_queue.empty())
      throw empty_queue{};
    value = m_queue.front();
    m_queue.pop();
  }
  std::shared_ptr<T> pop() {
    const std::lock_guard<std::mutex> lock{m_mutex};
    if (m_queue.empty())
      throw empty_queue{};
    auto result{std::make_shared<T>(m_queue.front())};
    m_queue.pop();
    return result;
  }

private:
  std::queue<T> m_queue;
  mutable std::mutex m_mutex;
};
