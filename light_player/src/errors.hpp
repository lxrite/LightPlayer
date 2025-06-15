/*
 *    errors.hpp:
 *
 *    Copyright (C) 2017-2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
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
        EndOfFile,
    };

    enum class DecodeErrors {
        Ok,
        Other,
        Again,
        OutOfMemory,
        EndOfFile,
    };

} // namespace lp

#endif // LIGHT_PLAYER_ERRORS_HPP
