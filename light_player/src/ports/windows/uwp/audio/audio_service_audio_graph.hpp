/*
 *    audio_service_audio_graph.hpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_AUDIO_SERVICE_AUDIO_GRAPH_HPP
#define LIGHT_PLAYER_AUDIO_SERVICE_AUDIO_GRAPH_HPP

#include "include/light_player/config.hpp"
#if LIGHT_PLAYER_PLATFORM == LIGHT_PLAYER_PLATFORM_WIN_UWP

#include <winrt/Windows.Media.Audio.h>

#include <functional>

#include "src/audio/audio_service.hpp"
#include "src/coroutine/task.hpp"

using lp::audio::AudioService;
using lp::coroutine::Task;
using winrt::Windows::Media::Audio::AudioDeviceOutputNode;
using winrt::Windows::Media::Audio::AudioFrameInputNode;
using winrt::Windows::Media::Audio::AudioGraph;

namespace lp::ports::windows::uwp::audio {

class AudioService_AudioGraph : public AudioService {
 public:
  AudioService_AudioGraph(AudioGraph graph, AudioFrameInputNode input_node,
                          AudioDeviceOutputNode output_node,
                          std::function<void()> audio_frame_completed);
  auto Start() -> void override;
  auto Stop() -> void override;
  auto WriteAudioFrameData(std::uint8_t *data, std::size_t size)
      -> void override;
  auto QueuedSampleCount() -> std::uint32_t const;
  auto QueuedFrameCount() -> std::uint32_t const override;

  static auto CreatePCMAudioService(std::uint32_t sample_rate,
                                    std::uint32_t channel_count,
                                    std::uint32_t bits_per_sample,
                                    std::function<void()> audio_frame_completed)
      -> Task<std::shared_ptr<AudioService_AudioGraph>>;

 private:
  AudioGraph audio_graph_;
  AudioFrameInputNode input_node_;
  AudioDeviceOutputNode output_node_;
  std::atomic<std::int32_t> queued_freme_count_ = {0};
};

}  // namespace lp::ports::windows::uwp::audio

#endif  // LIGHT_PLAYER_PLATFORM == LIGHT_PLAYER_PLATFORM_WIN_UWP

#endif  // LIGHT_PLAYER_AUDIO_SERVICE_AUDIO_GRAPH_HPP
