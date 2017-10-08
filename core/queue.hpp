/*
 *    queue.hpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_QUEUE_HPP
#define LIGHT_PLAYER_QUEUE_HPP

#include <list>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>	
}

#include "frame.hpp"
#include "packet.hpp"

namespace lp {

class PacketQueue {
public:
    PacketQueue();
    ~PacketQueue();

    auto Serial() const -> int;
    auto SerialPtr() const -> const std::shared_ptr<int>&;
    auto Push(const Packet& packet) -> void;
    auto Push(Packet&& packet) -> void;
    auto Pop() -> Packet;
    auto Empty() -> bool;
    auto Size() -> std::size_t;
    auto Flush() -> void;

private:
    std::list<Packet> packet_list;
    std::shared_ptr<int> serial;
};

class FrameQueue {
public:
    FrameQueue();
    ~FrameQueue();

    auto Push(const Frame& frame) -> void;
    auto Push(Frame&& frame) -> void;
    auto Pop() -> std::optional<Frame>;
    auto Empty() -> bool;
    auto Size() -> std::size_t;
    auto Peek() -> std::optional<Frame>;
    auto PeekNext() -> std::optional<Frame>;

private:
    std::list<Frame> frame_list;
};

}; // namespace lp

#endif // LIGHT_PLAYER_QUEUE_HPP
