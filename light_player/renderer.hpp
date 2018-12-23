/*
 *    renderer.hpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_RENDERER_HPP
#define LIGHT_PLAYER_RENDERER_HPP

#include <memory>

#include "types.hpp"

namespace lp
{

class Renderer {
public:
    virtual ~Renderer() noexcept {}
    virtual auto Render(PixelFormat pixel_format, const byte_t* data, std::uint32_t width, std::uint32_t height) noexcept -> void = 0;
};

} // namespace lp

#endif // LIGHT_PLAYER_RENDERER_HPP
