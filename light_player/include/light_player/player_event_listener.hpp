/*
 *    player_event_listener.hpp:
 *
 *    Copyright (C) 2017-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_PLAYER_EVENT_LISTENER_HPP
#define LIGHT_PLAYER_PLAYER_EVENT_LISTENER_HPP

#include "include/light_player/config.hpp"
#include "include/light_player/player_event_arg.hpp"

namespace lp {

class LIGHT_PLAYER_API PlayerEventListener {
 public:
  virtual ~PlayerEventListener(){};
  virtual auto OnPlayerStateOpening(const PlayerStateOpeningEventArg &event_arg)
      -> void = 0;
  virtual auto OnPlayerStateOpened(const PlayerStateOpenedEventArg &event_arg)
      -> void = 0;
  virtual auto OnPlayerStatePlaying(const PlayerStatePlayingEventArg &event_arg)
      -> void = 0;
  virtual auto OnPlayerStatePaused(const PlayerStatePausedEventArg &event_arg)
      -> void = 0;
  virtual auto OnPlayerStateMediaEnd(
      const PlayerStateMediaEndEventArg &event_arg) -> void = 0;
  virtual auto OnPlayerStateClosing(const PlayerStateClosingEventArg &event_arg)
      -> void = 0;
  virtual auto OnPlayerStateClosed(const PlayerStateClosedEventArg &event_arg)
      -> void = 0;
};

}  // namespace lp

#endif  // LIGHT_PLAYER_PLAYER_EVENT_LISTENER_HPP
