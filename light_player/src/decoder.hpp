/*
 *    decoder.hpp:
 *
 *    Copyright (C) 2017-2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_DECODER_HPP
#define LIGHT_PLAYER_DECODER_HPP

#include <memory>
#include <optional>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "src/errors.hpp"
#include "src/frame.hpp"
#include "src/packet.hpp"

namespace lp {

    class Decoder {
        explicit Decoder(AVCodecContext* codec_ctx);
    public:
        ~Decoder();

        auto SendPacket(const Packet& packet) -> DecodeErrors;
        auto ReceiveFrame(Frame& frame) -> DecodeErrors;
        auto SetPacketSerial(int packet_serial) -> void;
        auto PacketSerial() const -> std::optional<int>;
        auto GetPacketTimeBase() const -> AVRational;
        auto GetCodecContext() const -> AVCodecContext*;
        auto FlushBuffers() -> void;

        static auto OpenDecoder(const AVCodecParameters* codecpar, const AVCodec* codec, AVDictionary** options) -> std::shared_ptr<Decoder>;

    private:
        AVCodecContext* codec_ctx_;
        std::optional<int> packet_serial_;
    };

} // namespace lp

#endif // LIGHT_PLAYER_DECODER_HPP
