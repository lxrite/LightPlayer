/*
 *    queue.cpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "queue.hpp"

namespace lp {

PacketQueue::PacketQueue()
    : serial(std::make_shared<int>(0))
{}

PacketQueue::~PacketQueue()
{}

auto PacketQueue::Serial() const -> int
{
    return *serial;
}

auto PacketQueue::SerialPtr() const -> const std::shared_ptr<int>&
{
    return serial;
}

auto PacketQueue::Push(const Packet& packet) -> void
{
    packet_list.push_back(packet);
}

auto PacketQueue::Push(Packet&& packet) -> void
{
    packet_list.push_back(std::move(packet));
}

auto PacketQueue::Pop() -> Packet
{
    auto packet = std::move(packet_list.front());
    packet_list.pop_front();
    return packet;
}

auto PacketQueue::Empty() -> bool
{
    return packet_list.empty();
}

auto PacketQueue::Size() -> std::size_t
{
    return packet_list.size();
}

auto PacketQueue::Flush() -> void
{
    packet_list.clear();
    ++(*serial);
    auto packet = Packet::MakeFlushPacket();
    packet.SetSerial(*serial);
    packet_list.push_back(packet);
}

FrameQueue::FrameQueue()
{}

FrameQueue::~FrameQueue()
{}

auto FrameQueue::Push(const Frame& frame) -> void
{
    frame_list.push_back(frame);
}

auto FrameQueue::Push(Frame&& frame) -> void
{
    frame_list.push_back(std::move(frame));
}

auto FrameQueue::Pop() -> std::optional<Frame>
{
    if (frame_list.empty()) {
        return {};
    }
    else {
        auto frame = std::make_optional(std::move(frame_list.front()));
        frame_list.pop_front();
        return frame;
    }
}

auto FrameQueue::Empty() -> bool
{
    return frame_list.empty();
}

auto FrameQueue::Size()->std::size_t
{
    return frame_list.size();
}

auto FrameQueue::Peek() -> std::optional<Frame>
{
    if (frame_list.empty()) {
        return {};
    }
    else {
        return { frame_list.front() };
    }
}

auto FrameQueue::PeekNext() ->std::optional<Frame>
{
    if (frame_list.size() <= 1) {
        return {};
    }
    else {
        return { *std::next(frame_list.begin()) };
    }
}

} // namespace lp
