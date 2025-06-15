/*
 *    audio_service_winmm.hpp:
 *
 *    Copyright (C) 2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_AUDIO_SERVICE_WINMM_HPP
#define LIGHT_PLAYER_AUDIO_SERVICE_WINMM_HPP

#include "include/light_player/config.hpp"
#if LIGHT_PLAYER_PLATFORM == LIGHT_PLAYER_PLATFORM_WIN_DESKTOP

#include <windows.h>

#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

#include "src/audio/audio_service.hpp"
#include "src/coroutine/task.hpp"

using lp::coroutine::Task;
using lp::audio::AudioService;

namespace lp::ports::windows::win32::audio {

	struct AudioFrame {
		AudioFrame(std::uint8_t* data, std::size_t size) {
			this->data = std::unique_ptr<std::uint32_t[]>(new std::uint32_t[size]);
			std::memcpy(this->data.get(), data, size);
			std::memset(&wave_hdr, 0, sizeof(WAVEHDR));
			wave_hdr.lpData = reinterpret_cast<LPSTR>(this->data.get());
			wave_hdr.dwBufferLength = static_cast<DWORD>(size);
		}
		std::unique_ptr<std::uint32_t[]> data;
		WAVEHDR wave_hdr;
	};

	class AudioService_WinMM
		: public AudioService {
	public:
		AudioService_WinMM(HWAVEOUT wave_out_handle, HANDLE event_handle, std::function<void()> audio_frame_completed);
		~AudioService_WinMM();
		auto Start() -> void override;
		auto Stop() -> void override;
		auto WriteAudioFrameData(std::uint8_t* data, std::size_t size) -> void override;
		auto QueuedFrameCount() -> std::uint32_t const override;

		static auto CreatePCMAudioService(std::uint32_t sample_rate, std::uint32_t channel_count, std::uint32_t bits_per_sample, std::function<void()> audio_frame_completed) -> Task<std::shared_ptr<AudioService_WinMM>>;

	private:
		auto AudioThread(std::stop_token stoken) -> void;
		HWAVEOUT wave_out_handle_;
		HANDLE event_handle_;
		std::function<void()> audio_frame_completed_;
		std::mutex mutex_;
		std::optional<std::jthread> td_;
		std::deque<std::unique_ptr<AudioFrame>> audio_frame_queue_;
	};

} // namespace lp::ports::windows::win32::audio

#endif // LIGHT_PLAYER_PLATFORM == LIGHT_PLAYER_PLATFORM_WIN_DESKTOP

#endif // LIGHT_PLAYER_AUDIO_SERVICE_WINMM_HPP
