/*
 *    uwp_renderer_impl.cpp:
 *
 *    Copyright (C) 2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include <winrt/Windows.Storage.Streams.h>

#include <memory>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
}

#include "src/ports/windows/uwp/renderer/uwp_renderer_impl.hpp"

using namespace lp::ports::windows::uwp::renderer;
using namespace winrt::Windows::Storage::Streams;

UWPRenderImpl::UWPRenderImpl(std::shared_ptr<SwapChainPanelRenderer> renderer)
    : renderer_(renderer)
{
}

auto UWPRenderImpl::AddRef() -> std::int64_t
{
    return ++ref_count_;
}

auto UWPRenderImpl::Release() -> void
{
    auto cnt = --ref_count_;
    if (cnt <= 0) {
        delete this;
    }
}

auto UWPRenderImpl::Render(const Picture& picture) noexcept -> void
{
    auto width = picture.width();
    auto height = picture.height();
    if (picture.format() == AV_PIX_FMT_YUV420P) {
        const auto& yuv_data = picture.data();
        const auto& linesize = picture.linesize();
        const std::uint8_t* y_data = reinterpret_cast<const std::uint8_t*>(yuv_data[0]);
        const std::uint8_t* u_data = reinterpret_cast<const std::uint8_t*>(yuv_data[1]);
        const std::uint8_t* v_data = reinterpret_cast<const std::uint8_t*>(yuv_data[2]);

        auto y_linesize = linesize[0];
        auto uv_linesize = linesize[1];
        auto y_data_size = y_linesize * height;
        auto uv_data_size = uv_linesize * (height / 2);
        auto y_buffer = MakeBuffer(y_data, y_data_size);
        auto u_buffer = MakeBuffer(u_data, uv_data_size);
        auto v_buffer = MakeBuffer(v_data, uv_data_size);
        renderer_->RenderYUV420P(width, height, y_buffer, u_buffer, v_buffer, y_linesize, uv_linesize);
    }
    else {
        auto target_pix_fmt = AV_PIX_FMT_YUV420P;
        int align = 8;
        auto output_buffer_size_need = av_image_get_buffer_size(target_pix_fmt, width, height, align);
        if (scale_output_buffer_ == nullptr || scale_output_buffer_size_ < output_buffer_size_need) {
            scale_output_buffer_.reset(new std::uint8_t[output_buffer_size_need]);
            scale_output_buffer_size_ = output_buffer_size_need;
        }
        std::uint8_t* data[8] = { 0 };
        int linesize[8] = { 0 };
        av_image_fill_arrays(data, linesize, scale_output_buffer_.get(), target_pix_fmt, width, height, align);
        const auto& picture_data = picture.data();
        img_convert_ctx_ = sws_getCachedContext(img_convert_ctx_, width, height, static_cast<AVPixelFormat>(picture.format()), width, height, target_pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
        sws_scale(img_convert_ctx_, picture.data(), picture.linesize(), 0, height, data, linesize);

        auto y_linesize = linesize[0];
        auto uv_linesize = linesize[1];
        auto y_data_size = y_linesize * height;
        auto uv_data_size = uv_linesize * (height / 2);
        auto y_buffer = MakeBuffer(data[0], y_data_size);
        auto u_buffer = MakeBuffer(data[1], uv_data_size);
        auto v_buffer = MakeBuffer(data[2], uv_data_size);
        renderer_->RenderYUV420P(width, height, y_buffer, u_buffer, v_buffer, y_linesize, uv_linesize);
    }
}

auto UWPRenderImpl::MakeBuffer(const std::uint8_t* data, size_t size) -> IBuffer
{
    DataWriter data_writer;
    data_writer.WriteBytes(winrt::array_view<const unsigned char>(data, size));
    return data_writer.DetachBuffer();
}
