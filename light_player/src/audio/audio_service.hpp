/*
 *    audio_service.hpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_AUDIO_SERVICE_HPP
#define LIGHT_PLAYER_AUDIO_SERVICE_HPP

#include <functional>
#include <memory>

#include "src/coroutine/task.hpp"

using lp::coroutine::Task;

namespace lp::audio {

class AudioService {
 public:
  AudioService();
  virtual ~AudioService() = 0;

  virtual auto Start() -> void = 0;
  virtual auto Stop() -> void = 0;
  virtual auto WriteAudioFrameData(std::uint8_t *data, std::size_t size)
      -> void = 0;
  virtual auto QueuedFrameCount() -> std::uint32_t const = 0;
};

auto CreatePCMAudioService(std::uint32_t sample_rate,
                           std::uint32_t channel_count,
                           std::uint32_t bits_per_sample,
                           std::function<void()> audio_frame_completed)
    -> Task<std::shared_ptr<AudioService>>;

}  // namespace lp::audio

#endif  // LIGHT_PLAYER_AUDIO_SERVICE_HPP
