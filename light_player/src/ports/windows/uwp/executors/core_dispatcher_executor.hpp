/*
 *    core_dispatcher_executor.hpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_CORE_DISPATCHER_EXECUTOR_HPP
#define LIGHT_PLAYER_CORE_DISPATCHER_EXECUTOR_HPP

#include <winrt/Windows.UI.Core.h>

#include "include/light_player/executor.hpp"

namespace lp::ports::windows::uwp::executors {

class CoreDispatcherExecutor : public Executor {
 public:
  CoreDispatcherExecutor(winrt::Windows::UI::Core::CoreDispatcher dispatcher);

  virtual ~CoreDispatcherExecutor();

  auto AddRef() -> std::int64_t override;
  auto Release() -> void override;

  auto Dispatch(const Work &work) -> void override;

 private:
  std::atomic<std::int64_t> ref_count_ = {1};
  winrt::Windows::UI::Core::CoreDispatcher dispatcher_;
};

}  // namespace lp::ports::windows::uwp::executors

#endif  // LIGHT_PLAYER_CORE_DISPATCHER_EXECUTOR_HPP
