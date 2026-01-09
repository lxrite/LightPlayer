/*
 *    custom_io_wrapper.hpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_CUSTOM_IO_WRAPPER_HPP
#define LIGHT_PLAYER_CUSTOM_IO_WRAPPER_HPP

extern "C" {
#include <libavformat/avio.h>
}

#include "include/light_player/io/stream.hpp"
#include "include/light_player/shared_ptr.hpp"

namespace lp::io {

class CustomIOWrapper {
 public:
  CustomIOWrapper(IStream *stream);
  ~CustomIOWrapper();

  auto IOContext() const -> AVIOContext *;

 private:
  SharedPtr<IStream> stream_;
  AVIOContext *io_ctx_;
};

}  // namespace lp::io

#endif  // LIGHT_PLAYER_CUSTOM_IO_WRAPPER_HPP
