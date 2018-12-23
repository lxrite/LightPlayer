/*
 *    player_state.hpp:
 *
 *    Copyright (C) 2017-2018 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_PLAYER_STATE_HPP
#define LIGHT_PLAYER_PLAYER_STATE_HPP

namespace lp {

enum class PlayerState {
    Ready,
    Opening,
    Opended,
    Playing,
    Paused,
    Ended,
    Failed,
    Closing,
};

} // namespace lp

#endif // LIGHT_PLAYER_PLAYER_STATE_HPP
