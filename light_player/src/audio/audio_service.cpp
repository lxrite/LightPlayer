/*
 *    audio_service.cpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "src/audio/audio_service.hpp"

#include "include/light_player/config.hpp"
#include "src/ports/windows/uwp/audio/audio_service_audio_graph.hpp"
#include "src/ports/windows/win32/audio/audio_service_winmm.hpp"

using namespace lp::audio;

AudioService::AudioService() {}

AudioService::~AudioService() {}

auto lp::audio::CreatePCMAudioService(
    std::uint32_t sample_rate, std::uint32_t channel_count,
    std::uint32_t bits_per_sample, std::function<void()> audio_frame_completed)
    -> Task<std::shared_ptr<AudioService>> {
#if LIGHT_PLAYER_PLATFORM == LIGHT_PLAYER_PLATFORM_WIN_DESKTOP
  co_return co_await lp::ports::windows::win32::audio::AudioService_WinMM::
      CreatePCMAudioService(sample_rate, channel_count, bits_per_sample,
                            audio_frame_completed);
#elif LIGHT_PLAYER_PLATFORM == LIGHT_PLAYER_PLATFORM_WIN_UWP
  co_return co_await lp::ports::windows::uwp::audio::AudioService_AudioGraph::
      CreatePCMAudioService(sample_rate, channel_count, bits_per_sample,
                            audio_frame_completed);
#else
#error "Unsupported platform."
#endif
}
