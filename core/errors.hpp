/*
 *    errors.hpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_ERRORS_HPP
#define LIGHT_PLAYER_ERRORS_HPP

namespace lp {

enum class DemuxErrors {
    Ok,
    Other,
    Again,
    OutOfMemory,
};

enum class DecodeErrors {
    Ok,
    Other,
    Again,
    OutOfMemory,
};

} // namespace lp

#endif // LIGHT_PLAYER_ERRORS_HPP
