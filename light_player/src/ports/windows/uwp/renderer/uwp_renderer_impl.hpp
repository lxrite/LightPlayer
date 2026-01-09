/*
 *    uwp_renderer_impl.hpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_UWP_RENDER_IMPL_HPP
#define LIGHT_PLAYER_UWP_RENDER_IMPL_HPP

#include <atomic>
#include <cstdint>
#include <memory>

extern "C" {
#include <libswscale/swscale.h>
}

#include "include/light_player/renderer.hpp"
#include "src/ports/windows/uwp/renderer/swap_chain_panel_renderer.hpp"

using winrt::Windows::Storage::Streams::IBuffer;

namespace lp::ports::windows::uwp::renderer {

class UWPRenderImpl : public lp::Renderer {
 public:
  explicit UWPRenderImpl(std::shared_ptr<SwapChainPanelRenderer> renderer);
  virtual ~UWPRenderImpl() noexcept {}

  auto AddRef() -> std::int64_t override;
  auto Release() -> void override;

  auto Render(const Picture &picture) noexcept -> void override;

 private:
  auto MakeBuffer(const std::uint8_t *data, size_t size) -> IBuffer;

 private:
  std::atomic<std::int64_t> ref_count_ = {1};
  std::shared_ptr<SwapChainPanelRenderer> renderer_;
  SwsContext *img_convert_ctx_ = nullptr;
  std::unique_ptr<std::uint8_t[]> scale_output_buffer_;
  std::size_t scale_output_buffer_size_ = 0;
};

}  // namespace lp::ports::windows::uwp::renderer

#endif  // LIGHT_PLAYER_UWP_RENDER_IMPL_HPP
