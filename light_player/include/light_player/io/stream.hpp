/*
 *    stream.hpp:
 *
 *    Copyright (C) 2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_STREAM_HPP
#define LIGHT_PLAYER_STREAM_HPP

#include <cstddef>
#include <cstdint>

#include "include/light_player/config.hpp"
#include "include/light_player/ref_counted.hpp"

namespace lp::io {

	class LIGHT_PLAYER_API IStream
		: public RefCounted {
	public:
		virtual ~IStream() noexcept {}
		virtual auto CanSeek() const noexcept -> bool = 0;
		virtual auto Read(std::uint8_t* buf, std::size_t buf_size) noexcept -> int = 0;
		virtual auto Seek(std::int64_t offset, int whence) noexcept -> std::int64_t = 0;
	};

} // namespace lp::io

#endif // LIGHT_PLAYER_STREAM_HPP
