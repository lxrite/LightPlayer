/*
 *    executor.hpp:
 *
 *    Copyright (C) 2017-2018 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_EXECUTOR_HPP
#define LIGHT_PLAYER_EXECUTOR_HPP

#include "work.hpp"

namespace lp {

class Executor {
public:
    virtual ~Executor() {};

    virtual auto Dispatch(const Work& work) -> void = 0;
};

}// namespace lp

#endif // LIGHT_PLAYER_EXECUTOR_HPP
