/*
 *    executor.hpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_EXECUTOR_HPP
#define LIGHT_PLAYER_EXECUTOR_HPP

#include "work.hpp"

namespace lp {

class Executor {
public:
    virtual ~Executor() {};

    virtual auto Post(Work&& work) -> void = 0;
    virtual auto Post(const Work& work) -> void = 0;
};

}// namespace lp

#endif // LIGHT_PLAYER_EXECUTOR_HPP
