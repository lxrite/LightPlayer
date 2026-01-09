/*
 *    thread_pool.cpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "src/threading/thread_pool.hpp"

using namespace lp::threading;

ThreadPool::ThreadPool(size_t threadCount) {
  for (size_t i = 0; i < threadCount; ++i) {
    workers_.emplace_back([this](std::stop_token stoken) {
      while (!stoken.stop_requested()) {
        std::function<void()> task;
        {
          std::unique_lock lock(mutex_);
          cv_.wait(lock, stoken, [this] { return !tasks_.empty(); });

          if (stoken.stop_requested()) {
            break;
          }

          task = std::move(tasks_.front());
          tasks_.pop();
        }
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  for (auto &worker : workers_) {
    worker.request_stop();
  }
  cv_.notify_all();
  for (auto &worker : workers_) {
    worker.join();
  }
}

void ThreadPool::Schedule(Work task) {
  {
    std::lock_guard lg(mutex_);
    tasks_.push(std::move(task));
  }
  cv_.notify_one();
}
