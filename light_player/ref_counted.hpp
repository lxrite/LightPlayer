/*
 *    ref_counted.hpp:
 *
 *    Copyright (C) 2017-2018 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_REF_COUNTED_HPP
#define LIGHT_PLAYER_REF_COUNTED_HPP

#include <cstdint>

namespace lp {

class RefCounted {
public:
    virtual ~RefCounted() noexcept {};
    virtual auto AddRef() -> std::uint32_t = 0;
    virtual auto Release() -> void = 0;
};

} // namespace lp

#endif // LIGHT_PLAYER_REF_COUNTED_HPP
