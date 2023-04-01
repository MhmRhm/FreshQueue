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
    const std::lock_guard lock{m_mutex};
    return m_queue.size();
  }

  [[nodiscard]] bool empty() const noexcept {
    const std::lock_guard lock{m_mutex};
    return m_queue.empty();
  }

  void push(T val) {
    const std::lock_guard lock{m_mutex};
    m_queue.push(std::make_shared<T>(std::move(val)));
    m_pushNotification.notify_one();
  }

  void pop(T &value) {
    const std::lock_guard lock{m_mutex};
    if (m_queue.empty())
      throw empty_queue{};
    value = std::move(*m_queue.front());
    m_queue.pop();
  }

  std::shared_ptr<T> pop() {
    const std::lock_guard lock{m_mutex};
    if (m_queue.empty())
      throw empty_queue{};
    auto result{m_queue.front()};
    m_queue.pop();
    return result;
  }

  bool tryPop(T &value) {
    const std::lock_guard lock{m_mutex};
    if (m_queue.empty())
      return false;
    value = std::move(*m_queue.front());
    m_queue.pop();
    return true;
  }

  std::shared_ptr<T> tryPop() {
    const std::lock_guard lock{m_mutex};
    if (m_queue.empty())
      return {};
    auto result{m_queue.front()};
    m_queue.pop();
    return result;
  }

  void waitAndPop(T &value) {
    std::unique_lock uniqueLock{m_mutex};
    m_pushNotification.wait(uniqueLock, [&] { return !m_queue.empty(); });
    value = std::move(*m_queue.front());
    m_queue.pop();
    return;
  }

  std::shared_ptr<T> waitAndPop() {
    std::unique_lock uniqueLock{m_mutex};
    m_pushNotification.wait(uniqueLock, [&] { return !m_queue.empty(); });
    auto result{m_queue.front()};
    m_queue.pop();
    return result;
  }

private:
  std::queue<std::shared_ptr<T>> m_queue;
  mutable std::mutex m_mutex;
  std::condition_variable m_pushNotification;
};

template <typename T> class ConcurrentFreshQueue {
private:
  struct Node {
    std::shared_ptr<T> data;
    std::unique_ptr<Node> next;
  };

public:
  ConcurrentFreshQueue() : m_head{new Node{}}, m_tail{m_head.get()} {};
  ConcurrentFreshQueue(const ConcurrentFreshQueue &) = delete;
  ConcurrentFreshQueue(ConcurrentFreshQueue &&) noexcept = delete;
  ConcurrentFreshQueue &operator=(const ConcurrentFreshQueue &) = delete;
  ConcurrentFreshQueue &operator=(ConcurrentFreshQueue &&) noexcept = delete;
  virtual ~ConcurrentFreshQueue() = default;

private:
  Node *getTail() {
    const std::lock_guard tailLock{m_tailMutex};
    return m_tail;
  }

  std::unique_ptr<Node> popHead() {
    auto head{std::move(m_head)};
    m_head = std::move(head->next);
    return head;
  }

  std::unique_ptr<Node> tryPopHead() {
    const std::lock_guard headLock{m_headMutex};
    if (m_head.get() == getTail()) {
      return {};
    }
    return popHead();
  }

  bool tryPopHead(T &value) {
    const std::lock_guard headLock{m_headMutex};
    if (m_head.get() == getTail()) {
      return false;
    }
    value = std::move(*popHead()->data);
    return true;
  }

  std::unique_lock<std::mutex> waitForData() {
    std::unique_lock headLock{m_headMutex};
    m_pushNotification.wait(headLock,
                            [&] { return m_head.get() != getTail(); });
    return headLock;
  }

  std::unique_ptr<Node> waitPopHead() {
    std::unique_lock headLock{waitForData()};
    return popHead();
  }

  std::unique_ptr<Node> waitPopHead(T &value) {
    std::unique_lock headLock{waitForData()};
    value = std::move(*m_head->data);
    return popHead();
  }

public:
  void push(const T &value) {
    auto newTail{std::make_unique<Node>()};
    auto newTailRaw = newTail.get();
    auto newData = std::make_shared<T>(std::move(value));
    {
      std::lock_guard tailLock{m_tailMutex};
      m_tail->data = newData;
      m_tail->next = std::move(newTail);
      m_tail = newTailRaw;
    }
    m_pushNotification.notify_one();
  }

  std::shared_ptr<T> tryPop() {
    auto head{tryPopHead()};
    if (head) {
      return head->data;
    }
    return {};
  }

  bool tryPop(T &value) { return tryPopHead(value); }

  std::shared_ptr<T> waitAndPop() { return waitPopHead()->data; }

  void waitAndPop(T &value) { waitPopHead(value); }

  bool empty() {
    const std::lock_guard headLock{m_headMutex};
    return m_head.get() == getTail();
  }

private:
  std::unique_ptr<Node> m_head;
  Node *m_tail;
  std::mutex m_headMutex;
  std::mutex m_tailMutex;
  std::condition_variable m_pushNotification;
};
