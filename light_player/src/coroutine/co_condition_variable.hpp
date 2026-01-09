/*
 *    co_condition_variable.hpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_CO_CONDITION_VARIABLE_HPP
#define LIGHT_PLAYER_CO_CONDITION_VARIABLE_HPP

#include <coroutine>
#include <deque>
#include <mutex>

#include "src/coroutine/task.hpp"
#include "src/threading/thread_pool.hpp"

using lp::threading::ThreadPool;

namespace lp::coroutine {

class CoConditionVariable {
 public:
  CoConditionVariable() = default;

  template <typename Predicate>
  auto Wait(std::unique_lock<std::mutex> &lock, Predicate pred) -> Task<void> {
    while (!pred()) {
      co_await Wait(lock);
    }
  }

  auto Wait(std::unique_lock<std::mutex> &lock) {
    struct Awaiter {
      CoConditionVariable &self;
      std::unique_lock<std::mutex> &lock;

      auto await_ready() -> bool { return false; }

      auto await_suspend(std::coroutine_handle<> handle) -> void {
        std::lock_guard<std::mutex> lg(self.mutex_);
        self.waiters_.push_back(handle);
        lock.unlock();
      }

      auto await_resume() -> void { lock.lock(); }
    };

    return Awaiter{*this, lock};
  }

  auto NotifyAll() -> void {
    std::deque<std::coroutine_handle<>> resumed;
    {
      std::lock_guard<std::mutex> lg(mutex_);
      resumed.swap(waiters_);
    }
    if (!resumed.empty()) {
      ThreadPool::Shared().Schedule([resumed = std::move(resumed)]() {
        for (auto h : resumed) {
          h.resume();
        }
      });
    }
  }

  auto NotifyOne() -> void {
    std::optional<std::coroutine_handle<>> h;
    {
      std::lock_guard<std::mutex> lg(mutex_);
      if (!waiters_.empty()) {
        h = waiters_.front();
        waiters_.pop_front();
      }
    }
    if (h) {
      ThreadPool::Shared().Schedule([h = std::move(h)]() { h->resume(); });
    }
  }

 private:
  std::mutex mutex_;
  std::deque<std::coroutine_handle<>> waiters_;
};

}  // namespace lp::coroutine

#endif  // LIGHT_PLAYER_CO_CONDITION_VARIABLE_HPP
