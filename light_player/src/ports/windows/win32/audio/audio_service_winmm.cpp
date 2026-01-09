/*
 *    audio_service_winmm.cpp:
 *
 *    Copyright (C) 2025-2026 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "src/ports/windows/win32/audio/audio_service_winmm.hpp"

#if LIGHT_PLAYER_PLATFORM == LIGHT_PLAYER_PLATFORM_WIN_DESKTOP

using namespace lp::ports::windows::win32::audio;

AudioService_WinMM::AudioService_WinMM(
    HWAVEOUT wave_out_handle, HANDLE event_handle,
    std::function<void()> audio_frame_completed)
    : wave_out_handle_(wave_out_handle),
      event_handle_(event_handle),
      audio_frame_completed_(audio_frame_completed) {}

AudioService_WinMM::~AudioService_WinMM() {
  Stop();
  ::waveOutClose(wave_out_handle_);
  ::CloseHandle(event_handle_);
}

auto AudioService_WinMM::Start() -> void {
  td_ = std::jthread(
      [this](std::stop_token stoken) { this->AudioThread(stoken); });
}

auto AudioService_WinMM::Stop() -> void {
  if (td_) {
    td_->request_stop();
    ::SetEvent(event_handle_);
    td_->join();
    td_.reset();
  }
}

auto AudioService_WinMM::WriteAudioFrameData(std::uint8_t *data,
                                             std::size_t size) -> void {
  auto audio_frame = std::make_unique<AudioFrame>(data, size);
  {
    std::lock_guard<std::mutex> lg(mutex_);
    audio_frame_queue_.push_back(std::move(audio_frame));
  }
  ::SetEvent(event_handle_);
}

auto AudioService_WinMM::QueuedFrameCount() -> std::uint32_t const {
  std::lock_guard<std::mutex> lg(mutex_);
  return audio_frame_queue_.size();
}

auto AudioService_WinMM::CreatePCMAudioService(
    std::uint32_t sample_rate, std::uint32_t channel_count,
    std::uint32_t bits_per_sample, std::function<void()> audio_frame_completed)
    -> Task<std::shared_ptr<AudioService_WinMM>> {
  HWAVEOUT hwo = nullptr;
  WAVEFORMATEX wfx;
  std::memset(&wfx, 0, sizeof(wfx));
  wfx.wFormatTag = WAVE_FORMAT_PCM;
  wfx.nChannels = channel_count;
  wfx.nSamplesPerSec = sample_rate;
  wfx.wBitsPerSample = bits_per_sample;
  wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

  HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (hEvent == nullptr) {
    co_return nullptr;
  }

  auto mmres = waveOutOpen(&hwo, WAVE_MAPPER, &wfx, DWORD_PTR(hEvent),
                           DWORD_PTR(0), CALLBACK_EVENT);
  if (mmres != MMSYSERR_NOERROR) {
    ::CloseHandle(hEvent);
    co_return nullptr;
  }
  co_return std::make_shared<AudioService_WinMM>(hwo, hEvent,
                                                 audio_frame_completed);
}

auto AudioService_WinMM::AudioThread(std::stop_token stoken) -> void {
  std::uint32_t queued_frame_count = 0;
  while (true) {
    ::ResetEvent(event_handle_);
    std::uint32_t finished_frame_count = 0;
    {
      std::lock_guard<std::mutex> lg(mutex_);
      if (stoken.stop_requested()) {
        break;
      }
      while (queued_frame_count < audio_frame_queue_.size()) {
        auto audio_frame = audio_frame_queue_[queued_frame_count].get();
        ::waveOutPrepareHeader(wave_out_handle_, &(audio_frame->wave_hdr),
                               sizeof(WAVEHDR));
        ::waveOutWrite(wave_out_handle_, &(audio_frame->wave_hdr),
                       sizeof(WAVEHDR));
        queued_frame_count += 1;
      }
      for (std::uint32_t i = 0;
           i < static_cast<std::uint32_t>(audio_frame_queue_.size()); i++) {
        auto audio_frame = audio_frame_queue_[i].get();
        if (audio_frame->wave_hdr.dwFlags & WHDR_DONE) {
          ++finished_frame_count;
        } else {
          break;
        }
      }
      for (std::uint32_t i = 0; i < finished_frame_count; i++) {
        audio_frame_queue_.pop_front();
      }
      queued_frame_count = audio_frame_queue_.size();
    }
    if (finished_frame_count > 0) {
      if (audio_frame_completed_) {
        audio_frame_completed_();
      }
    }
    ::WaitForSingleObject(event_handle_, INFINITE);
  }
  ::waveOutReset(wave_out_handle_);
}

#endif  // LIGHT_PLAYER_PLATFORM == LIGHT_PLAYER_PLATFORM_WIN_DESKTOP
