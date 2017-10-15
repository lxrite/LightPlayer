/*
 *    renderer_event_listener.hpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_RENDERER_EVENT_LISTENER_HPP
#define LIGHT_PLAYER_RENDERER_EVENT_LISTENER_HPP

namespace lp {

class RendererEventListener {
public:
    virtual ~RendererEventListener() noexcept {};
    virtual auto OnSizeChanged() noexcept -> void = 0;
};

} // namespace lp

#endif // LIGHT_PLAYER_RENDERER_EVENT_LISTENER_HPP
