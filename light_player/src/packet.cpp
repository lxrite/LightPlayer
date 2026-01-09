/*
 *    packet.cpp:
 *
 *    Copyright (C) 2017-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "src/packet.hpp"

using namespace lp;
using namespace lp::impl;

RawPacketHost::RawPacketHost(RawPacketType *raw_packet) noexcept
    : raw_packet_(raw_packet) {}

RawPacketHost::~RawPacketHost() {
  if (raw_packet_ != nullptr) {
    av_packet_unref(raw_packet_);
    av_packet_free(&raw_packet_);
  }
}

auto RawPacketHost::RawPacketPtr() const -> RawPacketType * {
  return raw_packet_;
}

Packet::~Packet() {}

auto Packet::RawPacketPtr() const -> const RawPacketType * {
  return raw_packet_host_->RawPacketPtr();
}

auto Packet::SetSerial(int serial) noexcept -> void { serial_ = serial; }

auto Packet::Serial() const noexcept -> std::optional<int> { return serial_; }

auto Packet::IsFlushPacket() const -> bool {
  return packet_type_ == PacketType::Flush;
}

auto Packet::AttachRawPacket(RawPacketType *raw_pkt) -> Packet {
  auto packet = Packet{};
  packet.raw_packet_host_ = std::make_shared<impl::RawPacketHost>(raw_pkt);
  packet.packet_type_ = PacketType::Normal;
  return packet;
}

auto Packet::MakeFlushPacket() -> Packet {
  auto packet = Packet{};
  packet.packet_type_ = PacketType::Flush;
  return packet;
}

auto Packet::MakeNullPacket() -> Packet {
  auto packet = Packet{};
  auto raw_pkt = av_packet_alloc();
  if (raw_pkt == nullptr) {
    throw std::bad_alloc();
  }
  raw_pkt->data = nullptr;
  raw_pkt->size = 0;
  packet.raw_packet_host_ = std::make_shared<impl::RawPacketHost>(raw_pkt);
  packet.packet_type_ = PacketType::Normal;
  return packet;
}
