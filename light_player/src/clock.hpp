/*
 *    clock.hpp:
 *
 *    Copyright (C) 2017-2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_CLOCK_HPP
#define LIGHT_PLAYER_CLOCK_HPP

#include <cmath>
#include <optional>
#include <memory>

namespace lp {

    class Clock {
    public:
        explicit Clock(const std::shared_ptr<int>& queue_serial);
        ~Clock();

        auto SetClock(double pts, int serial) -> void;
        auto SetClockAt(double pts, int serial, double time) -> void;
        auto GetClock() const -> double;

        static auto GetTimeRelative() -> double;

    private:
        double pts_ = NAN;
        double pts_drift_ = NAN;
        double last_updated_ = NAN;
        double speed_ = 1.0;
        std::optional<int> serial_;
        bool paused_ = false;
        std::shared_ptr<int> queue_serial_;
    };

} // namespace lp

#endif // LIGHT_PLAYER_CLOCK_HPP
