#pragma once
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <queue>

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

  [[nodiscard]] bool empty() const noexcept {
    const std::lock_guard<std::mutex> lock{m_mutex};
    return m_queue.empty();
  }

  void push(T val) {
    const std::lock_guard<std::mutex> lock{m_mutex};
    m_queue.push(std::make_shared<T>(std::move(val)));
    m_conditionVariable.notify_one();
  }

  void pop(T &value) {
    const std::lock_guard<std::mutex> lock{m_mutex};
    if (m_queue.empty())
      throw empty_queue{};
    value = std::move(*m_queue.front());
    m_queue.pop();
  }

  std::shared_ptr<T> pop() {
    const std::lock_guard<std::mutex> lock{m_mutex};
    if (m_queue.empty())
      throw empty_queue{};
    auto result{m_queue.front()};
    m_queue.pop();
    return result;
  }

  bool tryPop(T &value) {
    const std::lock_guard<std::mutex> lock{m_mutex};
    if (m_queue.empty())
      return false;
    value = std::move(*m_queue.front());
    m_queue.pop();
    return true;
  }

  std::shared_ptr<T> tryPop() {
    const std::lock_guard<std::mutex> lock{m_mutex};
    if (m_queue.empty())
      return {};
    auto result{m_queue.front()};
    m_queue.pop();
    return result;
  }

  void waitAndPop(T &value) {
    std::unique_lock<std::mutex> uniqueLock{m_mutex};
    m_conditionVariable.wait(uniqueLock, [&] { return !m_queue.empty(); });
    value = std::move(*m_queue.front());
    m_queue.pop();
    return;
  }

  std::shared_ptr<T> waitAndPop() {
    std::unique_lock<std::mutex> uniqueLock{m_mutex};
    m_conditionVariable.wait(uniqueLock, [&] { return !m_queue.empty(); });
    auto result{m_queue.front()};
    m_queue.pop();
    return result;
  }

private:
  std::queue<std::shared_ptr<T>> m_queue;
  mutable std::mutex m_mutex;
  std::condition_variable m_conditionVariable;
};
