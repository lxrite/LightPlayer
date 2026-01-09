/*
 *    demuxer.hpp:
 *
 *    Copyright (C) 2017-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_DEMUXER_HPP
#define LIGHT_PLAYER_DEMUXER_HPP

extern "C" {
#include <libavformat/avformat.h>
}

#include "src/errors.hpp"
#include "src/packet.hpp"

namespace lp {

class Demuxer {
 public:
  explicit Demuxer(AVFormatContext *format_ctx);
  ~Demuxer();

  auto GetPacket(Packet &packet) -> DemuxErrors;

 private:
  AVFormatContext *format_ctx_;
};

}  // namespace lp

#endif  // LIGHT_PLAYER_DEMUXER_HPP
