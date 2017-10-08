/*
 *    packet.hpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_PACKET_HPP
#define LIGHT_PLAYER_PACKET_HPP

#include <memory>
#include <optional>

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace lp {

enum class PacketType {
    Normal,
    Flush,
};

namespace impl {

class RawPacketHost {
public:
    using RawPacketType = AVPacket;

    explicit RawPacketHost(RawPacketType* raw_packet) noexcept;
    RawPacketHost(const RawPacketHost& other) = delete;
    RawPacketHost(RawPacketHost&& other) = delete;
    auto operator=(const RawPacketHost& other) = delete;
    auto operator=(RawPacketHost&& other) = delete;

    ~RawPacketHost();

    auto RawPacketPtr() const->RawPacketType*;

private:
    RawPacketType* raw_packet_;
};

} // namespace impl

class Packet {
public:
    using RawPacketType = AVPacket;
    Packet() noexcept = default;
    Packet(const Packet& other) noexcept = default;
    Packet(Packet&& other) noexcept = default;
    auto operator=(const Packet& other) -> Packet& = default;
    auto operator=(Packet&& other) noexcept -> Packet& = default;

    ~Packet();

    auto RawPacketPtr() const -> const RawPacketType*;
    auto SetSerial(int serial) noexcept -> void;
    auto Serial() const noexcept -> std::optional<int>;

    auto IsFlushPacket() const -> bool;

    static auto AttachRawPacket(RawPacketType* raw_pkt) -> Packet;
    static auto MakeFlushPacket() -> Packet;

private:
    std::shared_ptr<impl::RawPacketHost> raw_packet_host_;
    std::optional<int> serial_;
    std::optional<PacketType> packet_type_;
};

} // namespace lp

#endif // LIGHT_PLAYER_PACKET_HPP
