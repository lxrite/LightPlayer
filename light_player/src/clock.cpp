/*
 *    clock.cpp:
 *
 *    Copyright (C) 2017-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "src/clock.hpp"

extern "C" {
#include <libavutil/time.h>
}

using namespace lp;

Clock::Clock(const std::shared_ptr<int> &queue_serial) {
  queue_serial_ = queue_serial;
  SetClock(NAN, -1);
}

Clock::~Clock() {}

auto Clock::SetClock(double pts, int serial) -> void {
  SetClockAt(pts, serial, GetTimeRelative());
}

auto Clock::SetClockAt(double pts, int serial, double time) -> void {
  pts_ = pts;
  last_updated_ = time;
  pts_drift_ = pts - time;
  serial_ = serial;
}

auto Clock::GetClock() const -> double {
  if (queue_serial_ == nullptr || std::isnan(last_updated_)) {
    return NAN;
  }
  if (paused_) {
    return pts_;
  }
  auto time = GetTimeRelative();
  return pts_drift_ + time - (time - last_updated_) * (1.0 - speed_);
}

auto Clock::GetTimeRelative() -> double {
  return av_gettime_relative() / 1000000.0;
}
