/*
 *    random_access_stream_proxy.hpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_RANDOM_ACCESS_STREAM_PROXY_HPP
#define LIGHT_PLAYER_RANDOM_ACCESS_STREAM_PROXY_HPP

#include <shcore.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/base.h>

#include "include/light_player/io/stream.hpp"

using winrt::Windows::Storage::Streams::IRandomAccessStream;

namespace lp::ports::windows::uwp::io {

class RandomAccessStreamProxy : public lp::io::IStream {
 public:
  RandomAccessStreamProxy(IRandomAccessStream ras);

  auto AddRef() -> std::int64_t override;
  auto Release() -> void override;

  auto CanSeek() const noexcept -> bool override;
  auto Read(std::uint8_t *buf, std::size_t buf_size) noexcept -> int override;
  auto Seek(std::int64_t offset, int whence) noexcept -> std::int64_t override;

 private:
  std::atomic<std::uint32_t> ref_count_ = {1};
  winrt::com_ptr<::IStream> stream_;
};

}  // namespace lp::ports::windows::uwp::io

#endif  // LIGHT_PLAYER_RANDOM_ACCESS_STREAM_PROXY_HPP
