/*
 *    demuxer.hpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_DEMUXER_HPP
#define LIGHT_PLAYER_DEMUXER_HPP

extern "C" {
#include <libavformat/avformat.h>
}

#include "errors.hpp"
#include "packet.hpp"

namespace lp {

class Demuxer {
public:
    explicit Demuxer(AVFormatContext* format_ctx);
    ~Demuxer();

    auto GetPacket(Packet& packet) -> DemuxErrors;

private:
    AVFormatContext* format_ctx_;
};

} // namespace lp

#endif // LIGHT_PLAYER_DEMUXER_HPP
