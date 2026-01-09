/*
 *    renderer.hpp:
 *
 *    Copyright (C) 2017-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_RENDERER_HPP
#define LIGHT_PLAYER_RENDERER_HPP

#include "include/light_player/picture.hpp"
#include "include/light_player/ref_counted.hpp"
#include "include/light_player/types.hpp"

namespace lp {

class Renderer : public RefCounted {
 public:
  virtual ~Renderer() noexcept {}
  virtual auto Render(const Picture &picture) noexcept -> void = 0;
};

}  // namespace lp

#endif  // LIGHT_PLAYER_RENDERER_HPP
