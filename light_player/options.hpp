/*
 *    options.hpp:
 *
 *    Copyright (C) 2017-2018 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_OPTIONS_HPP
#define LIGHT_PLAYER_OPTIONS_HPP

#include "types.hpp"

namespace lp {

    struct RenderOptions {
        PixelFormat pixel_format = PixelFormat::PIXEL_FORMAT_BGRA;
        int align = 1;
    };

} // namespace lp

#endif // LIGHT_PLAYER_OPTIONS_HPP
