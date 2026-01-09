/*
 *    frame.hpp:
 *
 *    Copyright (C) 2017-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_FRAME_HPP
#define LIGHT_PLAYER_FRAME_HPP

#include <cmath>
#include <memory>
#include <optional>

extern "C" {
#include <libavutil/frame.h>
}

namespace lp {

namespace impl {

class RawFrameHost {
 public:
  using RawFrameType = AVFrame;

  explicit RawFrameHost(RawFrameType *raw_frame) noexcept;
  RawFrameHost(const RawFrameHost &other) = delete;
  RawFrameHost(RawFrameHost &&other) = delete;
  auto operator=(const RawFrameHost &other) = delete;
  auto operator=(RawFrameHost &&other) = delete;

  ~RawFrameHost();

  auto RawFramePtr() const -> RawFrameType *;

 private:
  RawFrameType *raw_frame_;
};

}  // namespace impl

class Frame {
 public:
  using RawFrameType = impl::RawFrameHost::RawFrameType;
  Frame() noexcept = default;
  Frame(const Frame &other) noexcept = default;
  Frame(Frame &&other) noexcept = default;

  auto operator=(const Frame &other) noexcept -> Frame & = default;
  auto operator=(Frame &&other) noexcept -> Frame & = default;

  ~Frame();

  auto RawFramePtr() const noexcept -> RawFrameType *;
  auto SetSerial(int serial) noexcept -> void;
  auto Serial() const noexcept -> std::optional<int>;
  auto SetPts(double pts) noexcept -> void;
  auto Pts() const noexcept -> double;
  auto SetDuration(double duration) noexcept -> void;
  auto Duration() const noexcept -> double;

  static auto AttachRawFrame(RawFrameType *raw_frame) -> Frame;

 private:
  std::shared_ptr<impl::RawFrameHost> raw_frame_host_;
  std::optional<int> serial_;
  double pts_ = NAN;
  double duration_ = NAN;
};

}  // namespace lp

#endif  // LIGHT_PLAYER_FRAME_HPP
