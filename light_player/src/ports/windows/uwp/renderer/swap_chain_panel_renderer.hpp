/*
 *    swap_chain_panel_renderer.hpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_SWAP_CHAIN_PANEL_RENDERER_HPP
#define LIGHT_PLAYER_SWAP_CHAIN_PANEL_RENDERER_HPP

#include <d3d11_2.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Controls.h>

#include <cstdint>
#include <memory>

using winrt::Windows::Storage::Streams::IBuffer;
using winrt::Windows::UI::Xaml::Controls::SwapChainPanel;

namespace lp::ports::windows::uwp::renderer {

struct YUVVertex {
  DirectX::XMFLOAT4 pos;
  DirectX::XMFLOAT2 uv;
};

enum class StretchMode { UniformToFill, Uniform, Fill };

struct VideoFrameData {
  VideoFrameData(std::uint32_t width, std::uint32_t height, IBuffer y_data,
                 IBuffer u_data, IBuffer v_data, std::uint32_t y_stride,
                 std::uint32_t u_stride, std::uint32_t v_stride)
      : width(width),
        height(height),
        y_data(y_data),
        u_data(u_data),
        v_data(v_data),
        y_stride(y_stride),
        u_stride(u_stride),
        v_stride(v_stride) {}
  std::uint32_t width;
  std::uint32_t height;
  IBuffer y_data;
  IBuffer u_data;
  IBuffer v_data;
  std::uint32_t y_stride;
  std::uint32_t u_stride;
  std::uint32_t v_stride;
};

class SwapChainPanelRenderer
    : public std::enable_shared_from_this<SwapChainPanelRenderer> {
 public:
  SwapChainPanelRenderer(SwapChainPanel panel, StretchMode stretch_mode);

  virtual ~SwapChainPanelRenderer();

  auto RenderYUV420P(std::uint32_t width, std::uint32_t height, IBuffer ydata,
                     IBuffer udata, IBuffer vdata, std::uint32_t y_stride,
                     std::uint32_t uv_stride) -> void;
  auto DoRenderYUV420P(std::uint32_t width, std::uint32_t height, IBuffer ydata,
                       IBuffer udata, IBuffer vdata, std::uint32_t y_stride,
                       std::uint32_t uv_stride) -> void;
  auto TrimAndRelease() -> void;

 private:
  auto CreateTexture2D(std::uint32_t width, std::uint32_t height)
      -> winrt::com_ptr<ID3D11Texture2D>;
  auto CreateShaderResourceView(ID3D11Resource *resource)
      -> winrt::com_ptr<ID3D11ShaderResourceView>;
  auto AspectRatio() const -> float;
  auto Draw() -> void;
  auto DrawLastVideoFrame() -> void;
  auto BeginRender() -> void;
  auto EndRender() -> void;

 private:
  StretchMode stretch_mode_;
  winrt::Windows::UI::Xaml::Controls::SwapChainPanel panel_;
  winrt::Windows::UI::Core::CoreDispatcher dispatcher_;
  double panel_width_ = 0;
  double panel_height_ = 0;
  float panel_scale_x_ = 1.0;
  float panel_scale_y_ = 1.0;
  bool size_changed_ = false;
  winrt::com_ptr<ID3D11Device> device_;
  winrt::com_ptr<ID3D11DeviceContext> device_context_;
  winrt::com_ptr<ID3D11VertexShader> vs_;
  winrt::com_ptr<ID3D11PixelShader> ps_;
  winrt::com_ptr<ID3D11InputLayout> input_layout_;
  winrt::com_ptr<ID3D11Buffer> vb_;
  winrt::com_ptr<ID3D11Buffer> vscb_;
  winrt::com_ptr<IDXGISwapChain2> swap_chain_;
  winrt::com_ptr<ID3D11RenderTargetView> rtv_;
  winrt::com_ptr<ID3D11SamplerState> sampler_state_;
  winrt::event_token size_changed_token_;
  std::shared_ptr<VideoFrameData> last_frame_data_;
};

}  // namespace lp::ports::windows::uwp::renderer

#endif  // LIGHT_PLAYER_SWAP_CHAIN_PANEL_RENDERER_HPP
