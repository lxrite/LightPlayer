/*
 *    fire_and_forget.hpp:
 *
 *    Copyright (C) 2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_FIRE_AND_FORGET_HPP
#define LIGHT_PLAYER_FIRE_AND_FORGET_HPP

#include <coroutine>
#include <exception>

namespace lp::coroutine {

    struct FireAndForget {
        struct promise_type {
            auto get_return_object() noexcept -> FireAndForget {
                return {};
            }

            auto initial_suspend() noexcept -> std::suspend_never {
                return {};
            }

            auto final_suspend() noexcept -> std::suspend_never {
                return {};
            }

            auto return_void() noexcept -> void {
            }

            auto unhandled_exception() -> void {
                std::terminate();
            }
        };
    };

} // namespace lp::coroutine

#endif // LIGHT_PLAYER_FIRE_AND_FORGET_HPP
