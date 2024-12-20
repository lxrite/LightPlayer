#pragma once

#include <cstdint>

#include "config.hpp"

namespace lp {

class LIGHT_PLAYER_API Picture {
public:
    using data_type = std::uint8_t*[8];
    using linesize_type = int[8];

    Picture(void* data, bool borrow);
    Picture(const Picture& other);
    ~Picture();

    auto format() const -> int;
    auto data() const -> const data_type&;
    auto linesize() const -> const linesize_type&;
    auto width() const -> int;
    auto height() const -> int;

private:
    void* data_ = nullptr;
    bool borrow_ = false;
};

} // namespace lp
