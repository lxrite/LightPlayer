/*
 *    picture.cpp:
 *
 *    Copyright (C) 2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include <new>

#include "include/light_player/picture.hpp"

extern "C" {
#include <libavutil/frame.h>
}

namespace lp {

Picture::Picture(void* data, bool borrow)
    : data_(data)
    , borrow_(borrow)
{
}

Picture::Picture(const Picture& other)
{
    auto src_frame = reinterpret_cast<AVFrame*>(other.data_);
    auto new_frame = av_frame_alloc();
    if (new_frame == nullptr) {
        throw std::bad_alloc();
    }
    new_frame->format = src_frame->format;
    new_frame->width = src_frame->width;
    new_frame->height = src_frame->height;
    av_frame_get_buffer(new_frame, 32);
    if (av_frame_copy(new_frame, src_frame) < 0) {
        av_frame_free(&new_frame);
        throw std::bad_alloc();
    }
    av_frame_copy_props(new_frame, src_frame);
    data_ = new_frame;
    borrow_ = false;
}

Picture::~Picture()
{
    if (!borrow_) {
        auto frame = reinterpret_cast<AVFrame*>(data_);
        av_frame_free(&frame);
    }
}

auto Picture::format() const -> int
{
    return reinterpret_cast<AVFrame*>(data_)->format;
}

auto Picture::data() const -> const data_type&
{
    return reinterpret_cast<AVFrame*>(data_)->data;
}

auto Picture::linesize() const -> const linesize_type&
{
    return reinterpret_cast<AVFrame*>(data_)->linesize;
}

auto Picture::width() const -> int
{
    return reinterpret_cast<AVFrame*>(data_)->width;
}

auto Picture::height() const -> int
{
    return reinterpret_cast<AVFrame*>(data_)->height;
}

} // namespace lp
