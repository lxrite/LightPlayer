/*
 *    packet.cpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "packet.hpp"

namespace lp {

namespace impl {

RawPacketHost::RawPacketHost(RawPacketType* raw_packet) noexcept
    : raw_packet_(raw_packet)
{
}

RawPacketHost::~RawPacketHost()
{
    if (raw_packet_ != nullptr) {
        av_packet_unref(raw_packet_);
        av_packet_free(&raw_packet_);
    }
}

auto RawPacketHost::RawPacketPtr() const -> RawPacketType*
{
    return raw_packet_;
}

} // namespace impl

Packet::~Packet()
{
}

auto Packet::RawPacketPtr() const -> const RawPacketType*
{
    return raw_packet_host_->RawPacketPtr();
}

auto Packet::SetSerial(int serial) noexcept -> void
{
    serial_ = serial;
}

auto Packet::Serial() const noexcept -> std::optional<int>
{
    return serial_;
}

auto Packet::IsFlushPacket() const -> bool
{
    return packet_type_ == PacketType::Flush;
}

auto Packet::AttachRawPacket(RawPacketType* raw_pkt) -> Packet
{
    auto packet = Packet{};
    packet.raw_packet_host_ = std::make_shared<impl::RawPacketHost>(raw_pkt);
    packet.packet_type_ = PacketType::Normal;
    return packet;
}

auto Packet::MakeFlushPacket() -> Packet
{
    auto packet = Packet{};
    packet.packet_type_ = PacketType::Flush;
    return packet;
}

} // namespace lp
