﻿/*
 *    types.hpp:
 *
 *    Copyright (C) 2017-2018 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_TYPES_HPP
#define LIGHT_PLAYER_TYPES_HPP

#include <cstdint>

namespace lp {

struct Size {
    std::uint32_t width;
    std::uint32_t height;
};

using byte_t = unsigned char;

enum class PixelFormat
{
    PIXEL_FORMAT_YUV420P = 0,
    PIXEL_FORMAT_BGRA = 1,
};

enum class PlayerOperationResult {
    Done,
    Pending,
    Fail,
};

} // namespace lp

#endif // LIGHT_PLAYER_TYPES_HPP
