/*
 *    player.hpp:
 *
 *    Copyright (C) 2017-2018 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_PLAYER_HPP
#define LIGHT_PLAYER_PLAYER_HPP

#include <memory>
#include <string>

#include "config.hpp"
#include "executor.hpp"
#include "options.hpp"
#include "player_event_listener.hpp"
#include "player_state.hpp"
#include "renderer.hpp"

namespace lp {

class LIGHT_PLAYER_API Player {
public:
    explicit Player(Executor* ui_executor);
    ~Player();
    auto Open(const std::string& url) -> PlayerOperationResult;
    auto Play() -> PlayerOperationResult;
    auto Pause() -> PlayerOperationResult;
    auto Seek(std::uint64_t time_us) -> PlayerOperationResult;
    auto Close() -> PlayerOperationResult;

    auto GetState() const noexcept -> PlayerState;
    auto GetDuration() const noexcept -> std::int64_t;
    auto GetPosition() const noexcept -> std::int64_t;

    auto SetRenderer(Renderer* renderer) -> void;
    auto SetEventListener(PlayerEventListener* event_listener) -> void;

    auto SetRenderOptions(const RenderOptions& options) -> bool;

private:
    void* impl_;
};

} // namespace lp

#endif // LIGHT_PLAYER_PLAYER_HPP
