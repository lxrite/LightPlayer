/*
 *    frame.cpp:
 *
 *    Copyright (C) 2017-2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "src/frame.hpp"

using namespace lp;
using namespace lp::impl;

RawFrameHost::RawFrameHost(RawFrameType* raw_frame) noexcept
    : raw_frame_(raw_frame)
{
}

RawFrameHost::~RawFrameHost()
{
    if (raw_frame_ != nullptr) {
        av_frame_unref(raw_frame_);
        av_frame_free(&raw_frame_);
    }
}

auto RawFrameHost::RawFramePtr() const -> RawFrameType*
{
    return raw_frame_;
}

Frame::~Frame()
{
}

auto Frame::RawFramePtr() const noexcept -> RawFrameType*
{
    return raw_frame_host_->RawFramePtr();
}

auto Frame::SetSerial(int serial) noexcept -> void
{
    serial_ = serial;
}

auto Frame::Serial() const noexcept -> std::optional<int>
{
    return serial_;
}

auto Frame::SetPts(double pts) noexcept -> void
{
    pts_ = pts;
}

auto Frame::Pts() const noexcept -> double
{
    return pts_;
}

auto Frame::SetDuration(double duration) noexcept -> void
{
    duration_ = duration;
}

auto Frame::Duration() const noexcept -> double
{
    return duration_;
}

auto Frame::AttachRawFrame(RawFrameType* raw_frame) -> Frame
{
    auto frame = Frame{};
    frame.raw_frame_host_ = std::make_shared<impl::RawFrameHost>(raw_frame);
    return frame;
}

