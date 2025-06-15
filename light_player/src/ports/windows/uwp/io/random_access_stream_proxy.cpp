/*
 *    random_access_stream_proxy.cpp:
 *
 *    Copyright (C) 2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "src/ports/windows/uwp/io/random_access_stream_proxy.hpp"

#include <shcore.h>

extern "C" {
#include <libavutil/error.h>
}

using namespace lp::ports::windows::uwp::io;

RandomAccessStreamProxy::RandomAccessStreamProxy(IRandomAccessStream ras)
{
	HRESULT hr = CreateStreamOverRandomAccessStream(winrt::get_unknown(ras), __uuidof(::IStream), stream_.put_void());
	if (FAILED(hr)) {
		winrt::throw_hresult(hr);
	}
}

auto RandomAccessStreamProxy::AddRef() -> std::int64_t
{
	return ++ref_count_;
}

auto RandomAccessStreamProxy::Release() -> void
{
	auto cnt = --ref_count_;
	if (cnt <= 0) {
		delete this;
	}
}

auto RandomAccessStreamProxy::CanSeek() const noexcept -> bool
{
	return true;
}

auto RandomAccessStreamProxy::Read(std::uint8_t* buf, std::size_t buf_size) noexcept -> int
{
	ULONG bytesRead{ 0 };
		 
	HRESULT hr = stream_->Read(buf, buf_size, &bytesRead);
	if (FAILED(hr)) {
		return AVERROR_UNKNOWN;
	}
	if (bytesRead == 0) {
		return AVERROR_EOF;;
	}
	return bytesRead;
}

auto RandomAccessStreamProxy::Seek(std::int64_t offset, int whence) noexcept -> std::int64_t
{
	LARGE_INTEGER in{ 0 };
	in.QuadPart = offset;
	ULARGE_INTEGER out{ 0 };

	HRESULT hr = stream_->Seek(in, whence, &out);
	if (FAILED(hr)) {
		return AVERROR_UNKNOWN;
	}

	return out.QuadPart;
}
