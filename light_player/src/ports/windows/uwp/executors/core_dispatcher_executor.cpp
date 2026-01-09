/*
 *    core_dispatcher_executor.cpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "src/ports/windows/uwp/executors/core_dispatcher_executor.hpp"

#include <winrt/Windows.Foundation.h>

using namespace lp::ports::windows::uwp::executors;
using namespace winrt::Windows::UI::Core;

CoreDispatcherExecutor::CoreDispatcherExecutor(CoreDispatcher dispatcher)
    : dispatcher_(dispatcher) {}

CoreDispatcherExecutor::~CoreDispatcherExecutor() {}

auto CoreDispatcherExecutor::AddRef() -> std::int64_t { return ++ref_count_; }
auto CoreDispatcherExecutor::Release() -> void {
  auto cnt = --ref_count_;
  if (cnt <= 0) {
    delete this;
  }
}

auto CoreDispatcherExecutor::Dispatch(const Work &work) -> void {
  if (dispatcher_.HasThreadAccess()) {
    work();
  } else {
    DispatchedHandler handler([work]() { work(); });
    dispatcher_.RunAsync(CoreDispatcherPriority::High, handler).get();
  }
}
