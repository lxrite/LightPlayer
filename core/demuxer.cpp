/*
 *    demuxer.cpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "demuxer.hpp"

namespace lp {

Demuxer::Demuxer(AVFormatContext* format_ctx)
    : format_ctx_(format_ctx)
{
}

Demuxer::~Demuxer()
{
}

auto Demuxer::GetPacket(Packet& packet)-> DemuxErrors
{
    auto raw_pkt = av_packet_alloc();
    if (raw_pkt == nullptr) {
        return DemuxErrors::OutOfMemory;
    }
    auto temp_packet = Packet{};
    auto ret = av_read_frame(format_ctx_, raw_pkt);
    auto err = DemuxErrors::Other;
    switch (ret) {
    case 0:
         packet = std::move(Packet::AttachRawPacket(raw_pkt));
         raw_pkt = nullptr;
         err = DemuxErrors::Ok;
        break;
    case AVERROR_EOF:
        err = DemuxErrors::EndOfFile;
        break;
    default:
        break;
    }
    if (raw_pkt != nullptr) {
        av_packet_unref(raw_pkt);
        av_packet_free(&raw_pkt);
    }
    return err;
}

} // namespace lp
