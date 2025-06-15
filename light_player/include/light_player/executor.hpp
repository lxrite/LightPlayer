/*
 *    executor.hpp:
 *
 *    Copyright (C) 2017-2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_EXECUTOR_HPP
#define LIGHT_PLAYER_EXECUTOR_HPP

#include "include/light_player/config.hpp"
#include "include/light_player/ref_counted.hpp"
#include "include/light_player/work.hpp"

namespace lp {

    class LIGHT_PLAYER_API Executor
        : public RefCounted {
    public:
        virtual ~Executor() {};

        virtual auto Dispatch(const Work& work) -> void = 0;
    };

}// namespace lp

#endif // LIGHT_PLAYER_EXECUTOR_HPP
