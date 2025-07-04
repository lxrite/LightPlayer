﻿/*
 *    decoder.cpp:
 *
 *    Copyright (C) 2017-2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "src/decoder.hpp"

extern "C" {
#include "libavcodec/avcodec.h"
}

using namespace lp;

Decoder::Decoder(AVCodecContext* codec_ctx)
    : codec_ctx_(codec_ctx)
{
}
Decoder::~Decoder()
{
    avcodec_free_context(&codec_ctx_);
}

auto Decoder::SendPacket(const Packet& packet) -> DecodeErrors
{
    auto ret = avcodec_send_packet(codec_ctx_, packet.RawPacketPtr());
    auto err = DecodeErrors::Other;
    switch (ret) {
    case 0:
        err = DecodeErrors::Ok;
        break;
    default:
        break;
    }
    return err;
}

auto Decoder::ReceiveFrame(Frame& frame) -> DecodeErrors
{
    auto raw_frame = av_frame_alloc();
    if (raw_frame == nullptr) {
        return DecodeErrors::OutOfMemory;
    }
    auto ret = avcodec_receive_frame(codec_ctx_, raw_frame);
    auto err = DecodeErrors::Other;
    switch (ret) {
    case 0:
        if (codec_ctx_->codec_type == AVMEDIA_TYPE_VIDEO) {
            raw_frame->pts = raw_frame->best_effort_timestamp;
        }
        frame = Frame::AttachRawFrame(raw_frame);
        raw_frame = nullptr;
        err = DecodeErrors::Ok;
        break;
    case AVERROR(EAGAIN):
        err = DecodeErrors::Again;
        break;
    case AVERROR_EOF:
        err = DecodeErrors::EndOfFile;
        break;
    default:
        break;
    }
    if (raw_frame != nullptr) {
        av_frame_unref(raw_frame);
        av_frame_free(&raw_frame);
    }
    return err;
}

auto Decoder::SetPacketSerial(int packet_serial) -> void
{
    packet_serial_ = packet_serial;
}

auto Decoder::PacketSerial() const -> std::optional<int>
{
    return packet_serial_;
}

auto Decoder::GetPacketTimeBase() const -> AVRational
{
    return codec_ctx_->pkt_timebase;
}

auto Decoder::GetCodecContext() const -> AVCodecContext*
{
    return codec_ctx_;
}

auto Decoder::FlushBuffers() -> void
{
    avcodec_flush_buffers(codec_ctx_);
}

auto Decoder::OpenDecoder(const AVCodecParameters* codecpar, const AVCodec* codec, AVDictionary** options) -> std::shared_ptr<Decoder>
{
    auto codec_ctx = avcodec_alloc_context3(codec);
    if (codec_ctx == nullptr) {
        return { nullptr };
    }

    if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
        codec_ctx->thread_count = 0;
        if (codec->capabilities & AV_CODEC_CAP_FRAME_THREADS) {
            codec_ctx->thread_type = FF_THREAD_FRAME;
        }
        else if (codec->capabilities & AV_CODEC_CAP_SLICE_THREADS) {
            codec_ctx->thread_type = FF_THREAD_SLICE;
        }
        else {
            codec_ctx->thread_count = 1;
        }
    }

    if (avcodec_parameters_to_context(codec_ctx, codecpar) < 0) {
        avcodec_free_context(&codec_ctx);
        return { nullptr };
    }
    if (avcodec_open2(codec_ctx, codec, options) < 0) {
        avcodec_free_context(&codec_ctx);
        return { nullptr };
    }
    return std::shared_ptr<Decoder>(new Decoder(codec_ctx));
}
