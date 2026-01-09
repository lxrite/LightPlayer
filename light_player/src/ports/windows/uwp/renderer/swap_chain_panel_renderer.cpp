/*
 *    swap_chain_panel_renderer.cpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "src/ports/windows/uwp/renderer/swap_chain_panel_renderer.hpp"

#include <DirectXMath.h>
#include <robuffer.h>
#include <windows.ui.xaml.media.dxinterop.h>

#include "src/ports/windows/uwp/renderer/pixel_shader.h"
#include "src/ports/windows/uwp/renderer/vertex_shader.h"

using namespace lp::ports::windows::uwp::renderer;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;

SwapChainPanelRenderer::SwapChainPanelRenderer(SwapChainPanel panel,
                                               StretchMode stretch_mode)
    : panel_(panel),
      dispatcher_(panel.Dispatcher()),
      stretch_mode_(stretch_mode) {
  panel_width_ = panel.ActualWidth();
  panel_height_ = panel.ActualHeight();
  panel_scale_x_ = panel.CompositionScaleX();
  panel_scale_y_ = panel.CompositionScaleY();
  UINT creation_flags =
      D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_VIDEO_SUPPORT;

#if defined(_DEBUG)
  creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  const D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_2,
      D3D_FEATURE_LEVEL_9_1,
  };
  auto hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                              creation_flags, feature_levels,
                              ARRAYSIZE(feature_levels), D3D11_SDK_VERSION,
                              device_.put(), nullptr, device_context_.put());
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }

  winrt::com_ptr<ID3D10Multithread> mtc = device_.as<ID3D10Multithread>();
  mtc->SetMultithreadProtected(TRUE);
  hr = device_->CreateVertexShader(YUV_RENDER_VERTEX_SHADER,
                                   ARRAYSIZE(YUV_RENDER_VERTEX_SHADER), nullptr,
                                   vs_.put());
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }

  hr = device_->CreatePixelShader(YUV_RENDER_PIXEL_SHADER,
                                  ARRAYSIZE(YUV_RENDER_PIXEL_SHADER), nullptr,
                                  ps_.put());
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }

  D3D11_INPUT_ELEMENT_DESC elements[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(DirectX::XMFLOAT4),
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  hr = device_->CreateInputLayout(
      elements, ARRAYSIZE(elements), YUV_RENDER_VERTEX_SHADER,
      ARRAYSIZE(YUV_RENDER_VERTEX_SHADER), input_layout_.put());
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }

  YUVVertex vertices[] = {
      {DirectX::XMFLOAT4(-1, 1, .5f, 1), DirectX::XMFLOAT2(0, 0)},
      {DirectX::XMFLOAT4(1, 1, .5f, 1), DirectX::XMFLOAT2(1, 0)},
      {DirectX::XMFLOAT4(-1, -1, .5f, 1), DirectX::XMFLOAT2(0, 1)},
      {DirectX::XMFLOAT4(1, -1, .5f, 1), DirectX::XMFLOAT2(1, 1)},
  };
  D3D11_SUBRESOURCE_DATA vertices_sd;
  vertices_sd.pSysMem = vertices;
  vertices_sd.SysMemPitch = 0;
  vertices_sd.SysMemSlicePitch = 0;
  auto vbd = CD3D11_BUFFER_DESC(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER,
                                D3D11_USAGE_IMMUTABLE, 0, 0, sizeof(YUVVertex));
  hr = device_->CreateBuffer(&vbd, &vertices_sd, vb_.put());
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }
  auto vscbd =
      CD3D11_BUFFER_DESC(16, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC,
                         D3D11_CPU_ACCESS_WRITE, 0, 0);
  hr = device_->CreateBuffer(&vscbd, nullptr, vscb_.put());
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }

  D3D11_SAMPLER_DESC sampler_desc;
  sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampler_desc.MipLODBias = 0.0f;
  sampler_desc.MaxAnisotropy = 1;
  sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampler_desc.MinLOD = -FLT_MAX;
  sampler_desc.MaxLOD = FLT_MAX;

  hr = device_->CreateSamplerState(&sampler_desc, sampler_state_.put());
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }

  auto sampler_state = sampler_state_.get();
  device_context_->PSSetSamplers(0, 1, &sampler_state);

  size_changed_token_ = panel.SizeChanged(SizeChangedEventHandler(
      [this](IInspectable sender, SizeChangedEventArgs args) {
        panel_width_ = panel_.ActualWidth();
        panel_height_ = panel_.ActualHeight();
        size_changed_ = true;
        Draw();
      }));
}

SwapChainPanelRenderer::~SwapChainPanelRenderer() {}

auto fillFactors(StretchMode stretchMode, float factor,
                 float aspectRatioFactorToSource[]) -> void {
  switch (stretchMode) {
    case StretchMode::UniformToFill:
      aspectRatioFactorToSource[0] = factor > 1 ? 1.0f : 1 / factor;
      aspectRatioFactorToSource[1] = factor > 1 ? factor : 1.0f;
      break;
    case StretchMode::Fill:
      aspectRatioFactorToSource[0] = 1;
      aspectRatioFactorToSource[1] = 1;
      break;
    case StretchMode::Uniform:
      aspectRatioFactorToSource[0] = factor > 1 ? 1 / factor : 1.0f;
      aspectRatioFactorToSource[1] = factor > 1 ? 1.0f : factor;
      break;
  }
}

auto SwapChainPanelRenderer::RenderYUV420P(std::uint32_t width,
                                           std::uint32_t height, IBuffer ydata,
                                           IBuffer udata, IBuffer vdata,
                                           std::uint32_t y_stride,
                                           std::uint32_t uv_stride) -> void {
  if (dispatcher_.HasThreadAccess()) {
    DoRenderYUV420P(width, height, ydata, udata, vdata, y_stride, uv_stride);
  } else {
    dispatcher_.RunAsync(CoreDispatcherPriority::High,
                         [=, self = shared_from_this()]() {
                           this->DoRenderYUV420P(width, height, ydata, udata,
                                                 vdata, y_stride, uv_stride);
                         });
  }
}

auto SwapChainPanelRenderer::DoRenderYUV420P(
    std::uint32_t width, std::uint32_t height, IBuffer ydata, IBuffer udata,
    IBuffer vdata, std::uint32_t y_stride, std::uint32_t uv_stride) -> void {
  last_frame_data_ = std::make_shared<VideoFrameData>(
      width, height, ydata, udata, vdata, y_stride, uv_stride, uv_stride);
  Draw();
}

auto SwapChainPanelRenderer::TrimAndRelease() -> void {
  if (device_ != nullptr) {
    winrt::com_ptr<IDXGIDevice3> trimableDevice = device_.as<IDXGIDevice3>();
    trimableDevice->Trim();
    device_ = nullptr;
    device_context_ = nullptr;
  }
}

auto SwapChainPanelRenderer::CreateTexture2D(std::uint32_t width,
                                             std::uint32_t height)
    -> winrt::com_ptr<ID3D11Texture2D> {
  D3D11_TEXTURE2D_DESC texDesc;
  texDesc.Width = width;
  texDesc.Height = height;
  texDesc.MipLevels = 1;
  texDesc.ArraySize = 1;
  texDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8_UNORM;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
  texDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
  texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
  texDesc.MiscFlags = 0;
  winrt::com_ptr<ID3D11Texture2D> tex;
  HRESULT hr = device_->CreateTexture2D(&texDesc, nullptr, tex.put());
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }
  return tex;
}

auto SwapChainPanelRenderer::CreateShaderResourceView(ID3D11Resource *resource)
    -> winrt::com_ptr<ID3D11ShaderResourceView> {
  winrt::com_ptr<ID3D11ShaderResourceView> srv;
  HRESULT hr = device_->CreateShaderResourceView(resource, nullptr, srv.put());
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }
  return srv;
}

auto SwapChainPanelRenderer::AspectRatio() const -> float {
  return static_cast<float>(panel_width_ / panel_height_);
}

auto SwapChainPanelRenderer::Draw() -> void {
  if (device_ == nullptr || device_context_ == nullptr) {
    return;
  }
  BeginRender();
  DrawLastVideoFrame();
  EndRender();
}

auto SwapChainPanelRenderer::DrawLastVideoFrame() -> void {
  auto frame_data = last_frame_data_;
  if (frame_data == nullptr) {
    return;
  }
  auto width = frame_data->width;
  auto height = frame_data->height;
  auto y_data = frame_data->y_data.data();
  auto u_data = frame_data->u_data.data();
  auto v_data = frame_data->v_data.data();
  auto y_stride = frame_data->y_stride;
  auto u_stride = frame_data->u_stride;
  auto v_stride = frame_data->v_stride;

  winrt::com_ptr<ID3D11Texture2D> y_tex = CreateTexture2D(width, height);
  winrt::com_ptr<ID3D11Texture2D> u_tex =
      CreateTexture2D(width / 2, height / 2);
  winrt::com_ptr<ID3D11Texture2D> v_tex =
      CreateTexture2D(width / 2, height / 2);

  D3D11_MAPPED_SUBRESOURCE y_tex_mapped, u_tex_mapped, v_tex_mapped;
  auto hr = device_context_->Map(y_tex.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                                 &y_tex_mapped);
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }

  hr = device_context_->Map(u_tex.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                            &u_tex_mapped);
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }

  hr = device_context_->Map(v_tex.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                            &v_tex_mapped);
  if (FAILED(hr)) {
    winrt::throw_hresult(hr);
  }

  for (uint32_t y = 0; y < height; ++y) {
    memcpy(static_cast<uint8_t *>(y_tex_mapped.pData) +
               (y * y_tex_mapped.RowPitch),
           y_data + (y * y_stride), width);
  }
  for (uint32_t y = 0; y < height / 2; ++y) {
    memcpy(static_cast<uint8_t *>(u_tex_mapped.pData) +
               (y * u_tex_mapped.RowPitch),
           u_data + (y * u_stride), width / 2);
    memcpy(static_cast<uint8_t *>(v_tex_mapped.pData) +
               (y * v_tex_mapped.RowPitch),
           v_data + (y * v_stride), width / 2);
  }

  device_context_->Unmap(y_tex.get(), 0);
  device_context_->Unmap(u_tex.get(), 0);
  device_context_->Unmap(v_tex.get(), 0);

  auto ytex_srv = CreateShaderResourceView(y_tex.get());
  auto utex_srv = CreateShaderResourceView(u_tex.get());
  auto vtex_srv = CreateShaderResourceView(v_tex.get());

  auto factor =
      AspectRatio() / (static_cast<float>(width) / static_cast<float>(height));
  float factors[2];
  fillFactors(stretch_mode_, factor, factors);
  D3D11_MAPPED_SUBRESOURCE scale_buffer;
  device_context_->Map(vscb_.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                       &scale_buffer);
  memcpy(scale_buffer.pData, factors, sizeof(factors));
  device_context_->Unmap(vscb_.get(), 0);
  device_context_->VSSetShader(vs_.get(), nullptr, 0);
  device_context_->PSSetShader(ps_.get(), nullptr, 0);

  ID3D11ShaderResourceView *resource_views[] = {ytex_srv.get(), utex_srv.get(),
                                                vtex_srv.get()};
  device_context_->PSSetShaderResources(0, ARRAYSIZE(resource_views),
                                        resource_views);

  device_context_->IASetInputLayout(input_layout_.get());
  device_context_->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  UINT32 stride = sizeof(YUVVertex);
  UINT32 voffset = 0;
  auto vb = vb_.get();
  device_context_->IASetVertexBuffers(0, 1, &vb, &stride, &voffset);
  auto vscb = vscb_.get();
  device_context_->VSSetConstantBuffers(0, 1, &vscb);
  device_context_->Draw(4, 0);
}

auto SwapChainPanelRenderer::BeginRender() -> void {
  UINT render_width = static_cast<UINT>(panel_width_ * panel_scale_x_);
  UINT render_height = static_cast<UINT>(panel_height_ * panel_scale_y_);
  if (swap_chain_ == nullptr) {
    winrt::com_ptr<IDXGIDevice3> dxgi_device;
    auto hr =
        device_->QueryInterface(__uuidof(IDXGIDevice3), (void **)&dxgi_device);
    if (FAILED(hr)) {
      winrt::throw_hresult(hr);
    }
    winrt::com_ptr<IDXGIAdapter> dxgi_adapter;
    hr = dxgi_device->GetParent(__uuidof(IDXGIAdapter), (void **)&dxgi_adapter);
    if (FAILED(hr)) {
      winrt::throw_hresult(hr);
    }
    winrt::com_ptr<IDXGIFactory3> dxgi_factory;
    hr = dxgi_adapter->GetParent(__uuidof(IDXGIFactory3),
                                 (void **)&dxgi_factory);
    if (FAILED(hr)) {
      winrt::throw_hresult(hr);
    }
    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc;
    swap_chain_desc.Width = render_width;
    swap_chain_desc.Height = render_height;
    swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swap_chain_desc.Stereo = FALSE;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_IGNORE;
    swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
    swap_chain_desc.Flags = 0;

    winrt::com_ptr<IDXGISwapChain1> swap_chain1;
    hr = dxgi_factory->CreateSwapChainForComposition(
        device_.get(), &swap_chain_desc, nullptr, swap_chain1.put());
    if (FAILED(hr)) {
      winrt::throw_hresult(hr);
    }
    auto swap_chain2 = swap_chain1.as<IDXGISwapChain2>();

    DXGI_MATRIX_3X2_F inverse_scale = {0};
    inverse_scale._11 = 1.0 / panel_scale_x_;
    inverse_scale._22 = 1.0 / panel_scale_y_;

    swap_chain2->SetMatrixTransform(&inverse_scale);

    winrt::com_ptr<ID3D11Texture2D> back_buffer;
    hr = swap_chain2->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                back_buffer.put_void());
    if (FAILED(hr)) {
      winrt::throw_hresult(hr);
    };
    winrt::com_ptr<ID3D11RenderTargetView> rtv;
    hr = device_->CreateRenderTargetView(back_buffer.get(), nullptr, rtv.put());
    if (FAILED(hr)) {
      winrt::throw_hresult(hr);
    };
    panel_.as<ISwapChainPanelNative>()->SetSwapChain(swap_chain2.get());
    swap_chain_ = swap_chain2;
    rtv_ = rtv;
    size_changed_ = false;
  }
  if (size_changed_) {
    size_changed_ = false;
    device_context_->OMSetRenderTargets(0, nullptr, nullptr);
    rtv_ = nullptr;
    auto hr = swap_chain_->ResizeBuffers(2, render_width, render_height,
                                         DXGI_FORMAT_B8G8R8A8_UNORM, 0);
    if (FAILED(hr)) {
      winrt::throw_hresult(hr);
    }
    DXGI_MATRIX_3X2_F inverse_scale = {0};
    inverse_scale._11 = 1.0 / panel_scale_x_;
    inverse_scale._22 = 1.0 / panel_scale_y_;
    swap_chain_->SetMatrixTransform(&inverse_scale);

    winrt::com_ptr<ID3D11Texture2D> back_buffer;
    hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                back_buffer.put_void());
    if (FAILED(hr)) {
      winrt::throw_hresult(hr);
    };
    winrt::com_ptr<ID3D11RenderTargetView> rtv;
    hr = device_->CreateRenderTargetView(back_buffer.get(), nullptr, rtv.put());
    if (FAILED(hr)) {
      winrt::throw_hresult(hr);
    };
    rtv_ = rtv;
  }
  auto rtv = rtv_.get();
  device_context_->OMSetRenderTargets(1, &rtv, nullptr);
  D3D11_VIEWPORT vp;
  vp.Width = render_width;
  vp.Height = render_height;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  vp.MaxDepth = 1;
  vp.MinDepth = 0;
  device_context_->RSSetViewports(1, &vp);
  float clear_color[4] = {0.0, 0.0, 0.0, 0.0};
  device_context_->ClearRenderTargetView(rtv, clear_color);
}

auto SwapChainPanelRenderer::EndRender() -> void {
  if (swap_chain_ != nullptr) {
    swap_chain_->Present(0, 0);
  }
}
