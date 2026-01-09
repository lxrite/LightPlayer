/*
 *    custom_io_wrapper.cpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include <new>
#include <stdexcept>

extern "C" {
#include <libavutil/mem.h>
}

#include "src/io/custom_io_wrapper.hpp"

using namespace lp::io;

static auto ReadCallback(void *opaque, std::uint8_t *buf, int buf_size) -> int {
  auto stream = reinterpret_cast<lp::io::IStream *>(opaque);
  return stream->Read(buf, buf_size);
}

static auto SeekCallback(void *opaque, std::int64_t offset, int whence)
    -> std::int64_t {
  auto stream = reinterpret_cast<lp::io::IStream *>(opaque);
  return stream->Seek(offset, whence);
}

CustomIOWrapper::CustomIOWrapper(IStream *stream) : stream_(stream, true) {
  constexpr int buffer_size = 16 * 1024;
  auto io_buffer = reinterpret_cast<unsigned char *>(av_malloc(buffer_size));
  if (io_buffer == nullptr) {
    throw std::bad_alloc();
  }
  int64_t (*seek_callback)(void *opaque, int64_t offset, int whence) = nullptr;
  if (stream_->CanSeek()) {
    seek_callback = &SeekCallback;
  }
  AVIOContext *io_ctx =
      avio_alloc_context(io_buffer, buffer_size, 0, stream_.get(),
                         &ReadCallback, nullptr, seek_callback);
  if (io_ctx == nullptr) {
    // need free io_buffer?
    throw std::runtime_error("avio_alloc_context error.");
  }
  io_ctx_ = io_ctx;
}

auto CustomIOWrapper::IOContext() const -> AVIOContext * { return io_ctx_; }

CustomIOWrapper::~CustomIOWrapper() {}
