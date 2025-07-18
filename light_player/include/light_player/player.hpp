﻿/*
 *    player.hpp:
 *
 *    Copyright (C) 2017-2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_PLAYER_HPP
#define LIGHT_PLAYER_PLAYER_HPP

#include <memory>
#include <string>

#include "include/light_player/config.hpp"
#include "include/light_player/executor.hpp"
#include "include/light_player/player_event_listener.hpp"
#include "include/light_player/player_state.hpp"
#include "include/light_player/renderer.hpp"
#include "io/stream.hpp"

namespace lp {

    class LIGHT_PLAYER_API Player {
    public:
        explicit Player(Executor* ui_executor);
        ~Player();
        auto Open(const std::string& url) -> PlayerOperationResult;
        auto OpenStream(io::IStream* stream) -> PlayerOperationResult;
        auto Play() -> PlayerOperationResult;
        auto Pause() -> PlayerOperationResult;
        auto Seek(std::uint64_t time_us) -> PlayerOperationResult;
        auto Close() -> PlayerOperationResult;

        auto GetState() const noexcept -> PlayerState;
        auto GetDuration() const noexcept -> std::int64_t;
        auto GetPosition() const noexcept -> std::int64_t;

        auto SetRenderer(Renderer* renderer) -> void;
        auto SetEventListener(PlayerEventListener* event_listener) -> void;

    private:
        void* impl_;
    };

} // namespace lp

#endif // LIGHT_PLAYER_PLAYER_HPP
