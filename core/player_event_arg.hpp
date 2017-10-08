/*
 *    player_event_arg.hpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_PLAYER_EVENT_ARG_HPP
#define LIGHT_PLAYER_PLAYER_EVENT_ARG_HPP

namespace lp {

struct PlayerStateReadyEventArg {};

struct PlayerStateOpeningEventArg {};

struct PlayerStateOpenedEventArg {};

struct PlayerStatePlayingEventArg {};

struct PlayerStatePausedEventArg {};

struct PlayerStateClosingEventArg {};

struct PlayerStateClosedEventArg {};

} // namespace lp

#endif // LIGHT_PLAYER_PLAYER_EVENT_ARG_HPP
