#pragma once

#include <thread>
#include <shared_mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <optional>

#include "TaskStatus.h"

template <typename Ret>
class ThreadPool {
public:
  using task_t = std::function<Ret()>;
  using callback_t = std::function<void(TaskStatus<Ret>&&)>;

  ThreadPool(uint8_t nThreads) {
    while (nThreads--) {
      m_threads.push_back(std::thread(&ThreadPool<Ret>::routine, this));
    }
  }
  ~ThreadPool() {
    destroy();
  }

  void destroy(bool finishTasks = true) {
    std::unique_lock lock(m_queueMtx);

    if (m_destroyed) return;

    if (!finishTasks) {
      while (!m_queue.empty()) {
        m_queue.pop();
      }
    }

    m_destroyed = true;
    lock.unlock();

    m_taskCv.notify_all();

    for (auto& t : m_threads) {
      t.join();
    }

    m_threads.clear();
  }

  void addTask(task_t&& task, callback_t&& callback) {
    std::unique_lock lock(m_queueMtx);
    if (m_destroyed) return;

    callback({ Status::Pending });

    m_queue.emplace(std::forward<task_t>(task), std::forward<callback_t>(callback));
    lock.unlock();


    m_taskCv.notify_one();
  }
private:
  void routine() {
    while (true) {
      std::unique_lock lock(m_queueMtx);
      m_taskCv.wait(lock, [this] { return m_destroyed || !m_queue.empty(); });

      if (m_destroyed && m_queue.empty()) return;

      auto [task, callback] = m_queue.front();
      m_queue.pop();

      lock.unlock();

      callback({ Status::Processing });

      try {
        auto ret = task();
        callback({ Status::Fulfilled, ret });
      } catch (std::string err) {
        callback({ Status::Rejected, std::nullopt, err });
      }
    }
  }

  mutable std::shared_mutex m_queueMtx;
  mutable std::condition_variable_any m_taskCv;

  std::vector<std::thread> m_threads;
  std::queue<std::pair<task_t, callback_t>> m_queue;

  bool m_destroyed = false;
};
