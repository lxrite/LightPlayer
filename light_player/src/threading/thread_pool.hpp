/*
 *    thread_pool.hpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_THREAD_POOL_HPP
#define LIGHT_PLAYER_THREAD_POOL_HPP

#include <condition_variable>
#include <coroutine>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

#include "include/light_player/work.hpp"

using lp::Work;

namespace lp::threading {

class ThreadPool {
 public:
  ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
  ~ThreadPool();

  static auto Shared() -> ThreadPool & {
    static ThreadPool instance;
    return instance;
  }

  auto Schedule(Work task) -> void;

  auto Schedule() {
    struct ScheduleAwaitable {
      ThreadPool &pool;
      auto await_ready() const noexcept -> bool { return false; }

      void await_suspend(std::coroutine_handle<> handle) {
        pool.Schedule([handle]() { handle.resume(); });
      }

      void await_resume() const noexcept {}
    };
    return ScheduleAwaitable{*this};
  }

 private:
  std::vector<std::jthread> workers_;
  std::mutex mutex_;
  std::condition_variable_any cv_;
  std::queue<std::function<void()>> tasks_;
};

}  // namespace lp::threading

#endif  // LIGHT_PLAYER_THREAD_POOL_HPP
