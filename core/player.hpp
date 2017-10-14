/*
 *    player.hpp:
 *
 *    Copyright (C) 2017 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_PLAYER_HPP
#define LIGHT_PLAYER_PLAYER_HPP

#include <atomic>
#include <mutex>
#include <optional>
#include <thread>

extern "C" {
#include <libavformat/avformat.h>
}

#include "clock.hpp"
#include "executor.hpp"
#include "decoder.hpp"
#include "demuxer.hpp"
#include "player_state.hpp"
#include "player_event_listener.hpp"
#include "queue.hpp"
#include "renderer.hpp"

namespace lp {

enum class PlayerOperationResult {
    Done,
    Pending,
    Fail,
};

class Player : public std::enable_shared_from_this<Player> {
    Player(const std::shared_ptr<Executor>& ui_executor);
public:
    ~Player();
    auto Open(const std::string& url) -> PlayerOperationResult;
    auto Play() -> PlayerOperationResult;
    auto Pause() -> PlayerOperationResult;
    auto Seek(std::uint64_t time_us) -> PlayerOperationResult;
    auto Close() -> PlayerOperationResult;

    auto GetState() const noexcept -> PlayerState;

    auto SetRenderer(const std::shared_ptr<Renderer>& renderer) -> void;
    auto SetEventListener(const std::shared_ptr<PlayerEventListener>& event_listener) -> void;

    static auto CreateInstance(const std::shared_ptr<Executor>& ui_executor) -> std::shared_ptr<Player>;

private:
    auto DoOpen(const std::string& url) -> void;
    auto ReadThread() -> void;
    auto DecodeAudioThread() -> void;
    auto DecodeVideoThread() -> void;
    auto RenderThread() -> void;
    auto AudioThread() -> void;

private:
    auto VideoFrameDuration(const Frame& frame, const Frame& next_frame) const -> double;
    auto ComputeVideoFrameTargetDelay(double delay) const -> double;
    auto IsCloseRequested(bool lock) const -> bool;
    auto OnThreadExit() -> void;

private:
    PlayerState player_state_ = PlayerState::Ready;
    std::shared_ptr<Executor> ui_executor_;
    std::shared_ptr<PlayerEventListener> event_listener_;
    mutable std::mutex mutex_;
    int video_stream_index_ = -1;
    int audio_stream_index_ = -1;
    std::shared_ptr<Demuxer> demuxer_;
    std::shared_ptr<Decoder> video_decoder_;
    std::shared_ptr<Decoder> audio_decoder_;
    PacketQueue video_pkt_queue_;
    PacketQueue audio_pkt_queue_;
    std::condition_variable read_pkt_cv_;
    FrameQueue video_frame_queue_;
    FrameQueue audio_frame_queue_;
    std::condition_variable video_render_cv_;
    std::condition_variable audio_output_cv_;
    std::condition_variable video_decode_cv_;
    std::condition_variable audio_decode_cv_;
    std::shared_ptr<Clock> video_clock_;
    std::shared_ptr<Clock> audio_clock_;
    AVFormatContext* format_ctx_;
    std::shared_ptr<Renderer> renderer_;
    std::thread render_thread_;
    std::uint32_t video_width_;
    std::uint32_t video_height_;
    AVPixelFormat video_pix_fmt_;
    std::optional<Frame> last_show_frame_;
    double max_frame_duration_ = 10.0;
    double frame_timer_ = 0.0;
    bool paused_ = false;
    bool is_seek_requested_ = false;
    std::int64_t seek_target_us_ = 0;
    bool is_close_requested_ = false;
    std::atomic<int> running_thread_cnt_ = 0;
    bool is_end_of_file_ = false;
    bool is_end_of_video_stream_ = false;
    bool is_end_of_audio_stream_ = false;
};

} // namespace lp

#endif // LIGHT_PLAYER_PLAYER_HPP
