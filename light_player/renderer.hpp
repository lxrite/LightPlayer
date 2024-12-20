/*
 *    renderer.hpp:
 *
 *    Copyright (C) 2017-2018 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_RENDERER_HPP
#define LIGHT_PLAYER_RENDERER_HPP

#include "picture.hpp"
#include "ref_counted.hpp"
#include "types.hpp"

namespace lp
{

class Renderer
    : public RefCounted {
public:
    virtual ~Renderer() noexcept {}
    virtual auto Render(const Picture& picture) noexcept -> void = 0;
};

} // namespace lp

#endif // LIGHT_PLAYER_RENDERER_HPP
