/*
 *    renderer_event_listener_impl.cpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "renderer_event_listener_impl.hpp"

namespace lp::impl {

RendererEventListenerImpl::~RendererEventListenerImpl()
{}

auto RendererEventListenerImpl::OnSizeChanged() noexcept -> void
{
    if (OnSizeChangedListener) {
        OnSizeChangedListener();
    }
}

} // namespace lp::impl
