/*
 *    renderer_event_listener_impl.hpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_RENDERER_EVENT_LISTENER_IMPL_HPP
#define LIGHT_PLAYER_RENDERER_EVENT_LISTENER_IMPL_HPP

#include <functional>

#include "renderer_event_listener.hpp"

namespace lp::impl {

class RendererEventListenerImpl : public RendererEventListener {
public:
    ~RendererEventListenerImpl() noexcept;
    auto OnSizeChanged() noexcept -> void override;

    std::function<auto () -> void> OnSizeChangedListener;
};

} // namespace lp::impl



#endif // LIGHT_PLAYER_RENDERER_EVENT_LISTENER_IMPL_HPP
